#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

#include "misc.h"

#include "sysclk.h"
#include "tim2_delay.h"
#include "SSD1331.h"

#include "img.h"

void wait_and_clear() {
    delay_ms(1000);
    // clear display
    SSD1331_drawFrame(0,0,96,64, COLOR_BLACK, COLOR_BLACK);
    delay_us(500);
}

int main(void)
{
	char buffer[40];
	char i;
	const char *str_inFlash ="avislab.com/blog";

    SetSysClockToHSE();
    TIM2_init();

    // initialization
    SSD1331_init();

    wait_and_clear();

    // lines
    for (i = 0; i < 24; i++) {
    	SSD1331_drawLine(0, 0, i*4, 64, RGB(i*10,0,0));
    	SSD1331_drawLine(96, 0, i*4, 64, RGB(i*10,0,0));
    	delay_ms(20);
	}
    wait_and_clear();

    // circle
    SSD1331_drawCircle(48, 32, 5, COLOR_BLUE);
    delay_ms(50);
    SSD1331_drawCircle(48, 32, 10, COLOR_GOLDEN);
    delay_ms(50);
    SSD1331_drawCircle(48, 32, 15, COLOR_BLUE);
    delay_ms(50);
    SSD1331_drawCircle(48, 32, 20, COLOR_GOLDEN);
    delay_ms(50);
    SSD1331_drawCircle(48, 32, 25, COLOR_BLUE);
    delay_ms(50);
    SSD1331_drawCircle(48, 32, 30, COLOR_GOLDEN);
    wait_and_clear();

    // rect
    for (i = 0; i < 5; i++) {
    	SSD1331_drawFrame(i*5, i*5, 96-i*5, 64-i*5, COLOR_RED, COLOR_YELLOW);
    	delay_ms(20);
	}
    SSD1331_drawFrame(25, 25, 71, 39, COLOR_BLUE, COLOR_GREEN);
    wait_and_clear();

    // text FONT_1X
    SSD1331_SetXY(0,0);
    for (i = 33; i < 126; i++) {
    	SSD1331_Chr(FONT_1X, i, COLOR_BLUE, COLOR_BLACK);
    	SSD1331_XY_INK(FONT_1X);
	}
    wait_and_clear();

    // text FONT_2X
    SSD1331_SetXY(0,0);
    for (i = 33; i < 64; i++) {
    	SSD1331_Chr(FONT_2X, i, COLOR_BROWN, COLOR_BLACK);
    	SSD1331_XY_INK(FONT_2X);
	}
    wait_and_clear();

    // Numeric FONT_2X
    sprintf(buffer, "%s", "3758");
    SSD1331_SetXY(0, 0);
    SSD1331_Str(FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLACK);
    wait_and_clear();

    // Numeric FONT_4X
    SSD1331_SetXY(0, 0);
    SSD1331_Str(FONT_4X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLACK);
    wait_and_clear();

    // Images
    SSD1331_IMG(IMG0, 0,0, 96,64);
    wait_and_clear();

    SSD1331_IMG(IMG1, 16,0, 64,64);
    wait_and_clear();

    // copy window
    SSD1331_IMG(IMG3, 32,16, 32,32);
    SSD1331_copyWindow(32,16,64,48, 0,0);
    wait_and_clear();

    // scrolling
    SSD1331_IMG(IMG2, 23,7, 50,50);
    SSD1331_setScrolling(Horizontal, 0, 64, 1);
    SSD1331_enableScrolling(TRUE);

    wait_and_clear();
    SSD1331_enableScrolling(FALSE);
    // text from Flash
    SSD1331_SetXY(0, 24);
    SSD1331_FStr(FONT_1X,(unsigned char*)str_inFlash, COLOR_BLUE, COLOR_BLACK);

    while(1)
    {
    }
}
