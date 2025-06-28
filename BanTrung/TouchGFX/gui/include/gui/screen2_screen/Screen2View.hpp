#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <math.h>
#include "main.h"

#define GRID_HEIGHT		14
#define GRID_WIDTH		10

#define SCREEN_HEIGHT	320
#define SCREEN_WIDTH	240

#define LOW_THRESHOLD	312
#define HIGH_THRESHOLD	336

#define C_RADIUS		12
#define C_TWORADIUS		24

#define DEADLINE 		255

#define c_PI	3.1415926535f

#define ARROW_ANGLE		270

#define ARROW_SCALE_X	0.2f
#define ARROW_SCALE_Y	0.1f

#define BALL_X			120.f
#define BALL_Y			270.f
#define BALL_SPEED		20.f

#define EPS				0.0001f

struct Vector2D {
	float x, y;

	Vector2D() {}
	Vector2D(float _x, float _y) : x(_x), y(_y) {}

	static float deg2rad(float deg) { return deg / 180.f * c_PI; }
	static float rad2deg(float rad) { return rad / c_PI * 180.f; }

	Vector2D operator +(const Vector2D &other) const { return Vector2D(x + other.x, y + other.y); }
	Vector2D operator *(float k) const { return Vector2D(x * k, y * k); }

	float dot(const Vector2D &other) const { return x * other.x + y * other.y; }

	Vector2D rotate(float deg) const {
		float theta = deg2rad(deg);
		float cos_theta = cos(theta);
		float sin_theta = sin(theta);
		float nx = x * cos_theta + y * sin_theta;
		float ny = x * (-sin_theta) + y * cos_theta;
		return Vector2D(nx, ny);
	}

	Vector2D normalize() const {
		float dist = sqrt(x * x + y * y);
		return Vector2D(x / dist, y / dist);
	}
};

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void tickEvent() override;
protected:
    int get_x(const touchgfx::Circle &circle) const {
    	int x, y;
    	circle.getCenter(x, y);
    	return x;
    }
    int get_y(const touchgfx::Circle &circle) const {
    	int x, y;
		circle.getCenter(x, y);
		return y;
    }

    void reset_bfs() {
    	for(int i = 0; i < GRID_HEIGHT; ++i) {
			for(int j = 0; j < GRID_WIDTH; ++j) {
				visited[i][j] = false;
			}
		}
		queue_count = 0;
    }
    void add_queue(int i, int j, int org_i, int org_j, bool same_color = true);
    int bfs1(int i, int j);
    void bfs2();

    void init_circle(touchgfx::Circle &circle, int x, int y, const touchgfx::PainterRGB565 &painter);
    void fall_down(touchgfx::Circle &circle);

    uint32_t tick_count = 0;

    touchgfx::Circle circles_grid[GRID_HEIGHT][GRID_WIDTH];
    uint32_t color_state[GRID_HEIGHT][GRID_WIDTH];
    bool is_added[GRID_HEIGHT][GRID_WIDTH];

    uint32_t last_move;

    Vector2D up_arrow;
    int delta_angle = 0;
    bool is_aiming = false;

    bool is_flying = false;
    Vector2D fly_dir, ball_pos;
    uint32_t ball_state;

    int queue_i[GRID_HEIGHT * GRID_WIDTH];
    int queue_j[GRID_HEIGHT * GRID_WIDTH];
    int queue_count;
    bool visited[GRID_HEIGHT][GRID_WIDTH];

    uint32_t score = 0;
};

#endif // SCREEN2VIEW_HPP
