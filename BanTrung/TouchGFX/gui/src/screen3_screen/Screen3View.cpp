#include <gui/screen3_screen/Screen3View.hpp>
#include <stdio.h>
#include "cmsis_os.h"

static const uint16_t TEXTAREA4_SIZE = 50;
touchgfx::Unicode::UnicodeChar textArea4Buffer[TEXTAREA4_SIZE];

char s4[TEXTAREA4_SIZE];

extern osMessageQueueId_t Queue2Handle;

extern uint32_t final_score;

Screen3View::Screen3View()
{

}

void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();

    // Explosion signal
	uint8_t data = 'L';
	osMessageQueuePut(Queue2Handle, &data, 0, 10);

	snprintf(s4, TEXTAREA4_SIZE, "%u", final_score);
	Unicode::strncpy(textArea4Buffer, s4, TEXTAREA4_SIZE);
	scoreArea.setWildcard(textArea4Buffer);
	scoreArea.resizeToCurrentText();
	scoreArea.invalidate();

}

void Screen3View::tearDownScreen()
{
    Screen3ViewBase::tearDownScreen();
}
