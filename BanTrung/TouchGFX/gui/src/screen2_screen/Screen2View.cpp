#include <gui/screen2_screen/Screen2View.hpp>
#include <stdio.h>
#include "cmsis_os.h"

// For score display
static const uint16_t TEXTAREA_SIZE = 50;
touchgfx::Unicode::UnicodeChar textAreaBuffer[TEXTAREA_SIZE];

char s[TEXTAREA_SIZE];

uint32_t final_score;

uint32_t xorshift32()
{
    static uint32_t seed = 0;

    if (seed == 0)
        seed = HAL_GetTick();

    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;

    return seed;
}
// Queues message communicate
extern osMessageQueueId_t Queue1Handle;
extern osMessageQueueId_t Queue2Handle;


static inline float max(float x, float y) { return x < y ? y : x; }
static inline float min(float x, float y) { return x < y ? x : y; }

static float calc_dist(
		const Vector2D &p1,
		const Vector2D &p2) {
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) +
			(p1.y - p2.y) * (p1.y - p2.y));
}

static void solve_collision(
		const Vector2D &p1,
		const Vector2D &p2,
		const Vector2D &dir,
		float &t) {
	if(calc_dist(p1, p2) <= C_TWORADIUS + EPS) {
		t = 0;
		return;
	}
	float a = dir.x * dir.x + dir.y * dir.y;
	float b = 2 * dir.x + (p1.x - p2.x) +
			2 * dir.y * (p1.y - p2.y);
	float c = (p1.x - p2.x) * (p1.x - p2.x) +
			(p1.y - p2.y) * (p1.y - p2.y) - (C_TWORADIUS - EPS) * (C_TWORADIUS - EPS);
	float delta = b * b - 4 * a * c;
	if(delta < 0) {
		t = -1;
		return;
	}
	float sqrt_delta = sqrt(delta);
	if(-b - sqrt_delta >= 0) {
		t = (-b - sqrt_delta) / 2;
	}
	else if(-b + sqrt_delta >= 0) {
		t = (-b + sqrt_delta) / 2;
	}
	else {
		t = -1;
	}
}

Screen2View::Screen2View()
{

}

void Screen2View::init_circle(
		touchgfx::Circle &circle,
		int x, int y,
		const touchgfx::PainterRGB565 &painter)
{
	circle.setPosition(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	if(y >= LOW_THRESHOLD) {
		y = y - HIGH_THRESHOLD;
	}
	circle.setCenter(x, y);
	circle.setRadius(C_RADIUS);
	circle.setLineWidth(0);
	circle.setArc(0, 360);
	circle.setPainter(painter);
}

void Screen2View::add_queue(int i, int j, int org_i, int org_j, bool same_color) {
	if(i < 0) i += GRID_HEIGHT;
	else if(i >= GRID_HEIGHT) i -= GRID_HEIGHT;
	if(j < 0 || j >= GRID_WIDTH || !is_added[i][j] || visited[i][j])
		return;
	if(get_y(circles_grid[i][j]) >= DEADLINE - C_RADIUS)
		return;
	if(same_color && color_state[i][j] != color_state[org_i][org_j])
		return;
	queue_i[queue_count] = i;
	queue_j[queue_count] = j;
	++queue_count;
	visited[i][j] = true;
}

int Screen2View::bfs1(int si, int sj) {
	if(get_y(circles_grid[si][sj]) >= DEADLINE - C_RADIUS)
		return 0;
	reset_bfs();
	queue_i[queue_count] = si;
	queue_j[queue_count] = sj;
	visited[si][sj] = true;
	++queue_count;
	int ptr = 0;
	while(ptr < queue_count) {
		int i = queue_i[ptr], j = queue_j[ptr];
		++ptr;
		add_queue(i, j - 1, i, j);
		add_queue(i, j + 1, i, j);
		if(i & 1) {
			add_queue(i - 1, j, i, j);
			add_queue(i - 1, j + 1, i, j);
			add_queue(i + 1, j, i, j);
			add_queue(i + 1, j + 1, i, j);
		}
		else {
			add_queue(i - 1, j, i, j);
			add_queue(i - 1, j - 1, i, j);
			add_queue(i + 1, j, i, j);
			add_queue(i + 1, j - 1, i, j);
		}

	}
	return ptr;
}

void Screen2View::bfs2() {
	int lowest_row = -1;
	int lowest_value = 10000;
	for(int i = 0; i < GRID_HEIGHT; ++i) {
		int y = get_y(circles_grid[i][0]);
		if(lowest_value > y) {
			lowest_value = y;
			lowest_row = i;
		}
	}

	reset_bfs();

	for(int j = 0; j < GRID_WIDTH - (lowest_row & 1); ++j) {
		if(!is_added[lowest_row][j]) {
			continue;
		}
		visited[lowest_row][j] = true;
		queue_i[queue_count] = lowest_row;
		queue_j[queue_count] = j;
		++queue_count;
	}

	int ptr = 0;
	while(ptr < queue_count) {
		int i = queue_i[ptr], j = queue_j[ptr];
		++ptr;
		add_queue(i, j - 1, i, j, false);
		add_queue(i, j + 1, i, j, false);
		if(i & 1) {
			add_queue(i - 1, j, i, j, false);
			add_queue(i - 1, j + 1, i, j, false);
			add_queue(i + 1, j, i, j, false);
			add_queue(i + 1, j + 1, i, j, false);
		}
		else {
			add_queue(i - 1, j, i, j, false);
			add_queue(i - 1, j - 1, i, j, false);
			add_queue(i + 1, j, i, j, false);
			add_queue(i + 1, j - 1, i, j, false);
		}

	}
}

void Screen2View::fall_down(touchgfx::Circle &circle) {
	int x, y;
	circle.getCenter(x, y);
	++y;
	if(y >= LOW_THRESHOLD) {
		y = y - HIGH_THRESHOLD;
	}
	circle.setCenter(x, y);
}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    snprintf(s, TEXTAREA_SIZE, "%u", 0u);
    Unicode::strncpy(textAreaBuffer, s, TEXTAREA_SIZE);
    scoreArea.setWildcard(textAreaBuffer);
    scoreArea.invalidate();

    for(int i = 0; i < GRID_HEIGHT; ++i) {
		for(int j = 0; j < GRID_WIDTH - (i & 1); ++j) {
			uint32_t random = xorshift32();
			color_state[i][j] = random % 3;
			init_circle(circles_grid[i][j],
					C_TWORADIUS * j + C_RADIUS + ((i & 1) ? C_RADIUS : 0), C_TWORADIUS * i + C_RADIUS,
					color_state[i][j] == 0 ? redPainter : color_state[i][j] == 1 ? greenPainter : yellowPainter);
			if(get_y(circles_grid[i][j]) <= C_RADIUS) {
				add(circles_grid[i][j]);
				is_added[i][j] = true;
			}
			else {
				is_added[i][j] = false;
			}
		}
    }

    up_arrow = Vector2D(0.f, -1.f);
    arrow_svg.setRotation(ARROW_ANGLE);
    arrow_svg.setScaleX(ARROW_SCALE_X / 2);

    uint32_t random = xorshift32();
    ball_state = random % 3;
    bullet_ball.setPainter(ball_state == 0 ? redPainter : ball_state == 1 ? greenPainter : yellowPainter);

    uint8_t data = 'O';
    osMessageQueuePut(Queue2Handle, &data, 0, 10);
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::tickEvent() {
	if(is_flying) {
		float dist_rem = BALL_SPEED;
		bool flag_hit = false;
		while(dist_rem > 0) {
			float fly_dist = dist_rem;
			if(fly_dir.x > 0) {
				fly_dist = min(fly_dist, (SCREEN_WIDTH - ball_pos.x - C_RADIUS) / fly_dir.x);
			}
			else if(fly_dir.x < 0) {
				fly_dist = min(fly_dist, (ball_pos.x - C_RADIUS) / (-fly_dir.x));
			}
			for(int i = 0; i < GRID_HEIGHT; ++i) {
				for(int j = 0; j < GRID_WIDTH - (i & 1); ++j) {
					if(!is_added[i][j]) {
						continue;
					}
					int x, y;
					circles_grid[i][j].getCenter(x, y);
					Vector2D circle_center = Vector2D(float(x), float(y));
					float t;
					solve_collision(ball_pos, circle_center, fly_dir, t);
					if(t >= 0) {
						fly_dist = min(fly_dist, t);
					}
				}
			}
			fly_dist = max(fly_dist, 0.f);
			ball_pos = ball_pos + fly_dir * fly_dist;
			dist_rem -= fly_dist;

			// Find nearest ball
			bool hit_ball = false;
			for(int i = 0; i < GRID_HEIGHT; ++i) {
				for(int j = 0; j < GRID_WIDTH - (i & 1); ++j) {
					if(!is_added[i][j]) {
						continue;
					}
					int x, y;
					circles_grid[i][j].getCenter(x, y);
					float dist = sqrt(
							(x - ball_pos.x) * (x - ball_pos.x) +
							(y - ball_pos.y) * (y - ball_pos.y));
					if(dist <= C_TWORADIUS + EPS) {
						hit_ball = true;
						break;
					}
				}
				if(hit_ball) break;
			}

			if(hit_ball) {
				int nearest_i = -1, nearest_j = -1;
				float nearest_value = 10000.f;
				for(int i = 0; i < GRID_HEIGHT; ++i) {
					for(int j = 0; j < GRID_WIDTH - (i & 1); ++j) {
						if(is_added[i][j]) {
							continue;
						}
						int x, y;
						circles_grid[i][j].getCenter(x, y);
						float dist = sqrt(
								(x - ball_pos.x) * (x - ball_pos.x) +
								(y - ball_pos.y) * (y - ball_pos.y));
						if(dist < nearest_value) {
							nearest_value = dist;
							nearest_i = i;
							nearest_j = j;
						}
					}
				}
				if(nearest_i >= 0 && nearest_j >= 0) {
					is_added[nearest_i][nearest_j] = true;
					color_state[nearest_i][nearest_j] = ball_state % 3;
					circles_grid[nearest_i][nearest_j].setPainter(
							color_state[nearest_i][nearest_j] == 0 ?
									redPainter : color_state[nearest_i][nearest_j] == 1 ? greenPainter : yellowPainter);
					add(circles_grid[nearest_i][nearest_j]);



					int explo_count = bfs1(nearest_i, nearest_j);
					if(explo_count >= 3) {
						for(int i = 0; i < GRID_HEIGHT; ++i) {
							for(int j = 0; j < GRID_WIDTH; ++j) {
								if(visited[i][j] && is_added[i][j]) {
									remove(circles_grid[i][j]);
									is_added[i][j] = false;
								}
							}
						}
						int score_gain = explo_count;
						bfs2();
						for(int i = 0; i < GRID_HEIGHT; ++i) {
							for(int j = 0; j < GRID_WIDTH; ++j) {
								if(!visited[i][j] && is_added[i][j]) {
									remove(circles_grid[i][j]);
									is_added[i][j] = false;
									++score_gain;
								}
							}
						}

						score += score_gain;

						final_score = score;
						snprintf(s, TEXTAREA_SIZE, "%u", score);
							Unicode::strncpy(textAreaBuffer, s, TEXTAREA_SIZE);
						scoreArea.resizeToCurrentText();

						uint8_t data = 'E';
						osMessageQueuePut(Queue2Handle, &data, 0, 10);

					}
				}
				flag_hit = true;
				break;
			}

			if(ball_pos.x <= C_RADIUS + EPS || ball_pos.x >= SCREEN_WIDTH - C_RADIUS - EPS) {
				fly_dir.x = -fly_dir.x;
			}
			else if(ball_pos.y <= -C_RADIUS + EPS) {
				break;
			}
		}

		if(ball_pos.y <= -C_RADIUS + EPS || flag_hit) {
			is_flying = false;
			bullet_ball.setCenter(BALL_X, BALL_Y);
			uint32_t random = xorshift32();
			ball_state = random % 3;
			bullet_ball.setPainter(ball_state == 0 ? redPainter : ball_state == 1 ? greenPainter : yellowPainter);
		}
		else {
			bullet_ball.setCenter(ball_pos.x, ball_pos.y);
		}
	}
	else {
		bullet_ball.setCenter(BALL_X, BALL_Y);
	}

	uint8_t count = osMessageQueueGetCount(Queue1Handle);
	uint8_t res = 0;
	while(count > 0) {
		if (osMessageQueueGet(Queue1Handle, &res, NULL, 0) == osOK){
		if(res == 'L' && ARROW_ANGLE - delta_angle - 1 > 180) {
			++delta_angle;
			last_move = tick_count;
		}
		if(res == 'R' && ARROW_ANGLE - delta_angle + 1 < 360) {
			--delta_angle;
			last_move = tick_count;
		}
		if(res == 'X' && !is_flying && tick_count > 0) {
			is_flying = true;
			fly_dir = up_arrow.rotate(float(delta_angle));
			fly_dir = fly_dir.normalize();
			ball_pos = Vector2D(BALL_X, BALL_Y);
		}
		arrow_svg.setRotation(ARROW_ANGLE - delta_angle);
		count = osMessageQueueGetCount(Queue1Handle);
	}
	}

	if(tick_count - last_move <= 8 && !is_aiming) {
		is_aiming = true;
		arrow_svg.setScaleX(ARROW_SCALE_X);
	}
	else if(tick_count - last_move > 8 && is_aiming) {
		is_aiming = false;
		arrow_svg.setScaleX(ARROW_SCALE_X / 2);
	}

	const int N_BIT = 3;
	const int N = (1 << N_BIT) - 1;

	if(tick_count > 0 && (tick_count & N) == 0) {
		for(int i = 0; i < GRID_HEIGHT; ++i) {
			for(int j = 0; j < GRID_WIDTH; ++j) {
				fall_down(circles_grid[i][j]);
				if(get_y(circles_grid[i][j]) <= C_RADIUS && !is_added[i][j]) {
					color_state[i][j] = xorshift32() % 3;
					circles_grid[i][j].setPainter(
								color_state[i][j] == 0 ?
								redPainter : color_state[i][j] == 1 ? greenPainter : yellowPainter);
					add(circles_grid[i][j]);
					is_added[i][j] = true;
				}
				else if(get_y(circles_grid[i][j]) + C_RADIUS >= DEADLINE && is_added[i][j]) {
					lose_screen();
					uint8_t data = 'L';
					osMessageQueuePut(Queue2Handle, &data, 0, 10);
					remove(circles_grid[i][j]);
					is_added[i][j] = false;
				}
			}
		}
	}

	++tick_count;
	invalidate();
}
