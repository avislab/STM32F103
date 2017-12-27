#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

#include "stdio.h"
#include "misc.h"

#include "SSD1331.h"

static unsigned char CHR_X, CHR_Y;

void _sendCmd(uint8_t c)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_0); //dc
	GPIO_ResetBits(GPIOB, GPIO_Pin_1); //cs
	_sendData(c);
	GPIO_SetBits(GPIOB, GPIO_Pin_1); //cs
}

void _sendData(uint8_t c) {
	SPI_I2S_SendData(SPI1, c);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET) {
	}
}

void SSD1331_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	// Enable PORTB Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 |  GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
	GPIO_SetBits(GPIOA, GPIO_Pin_6);

	//SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1, ENABLE);
	SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);

    _sendCmd(CMD_DISPLAY_OFF);          //Display Off
    _sendCmd(CMD_SET_CONTRAST_A);       //Set contrast for color A
    _sendCmd(0x91);                     //145 (0x91)
    _sendCmd(CMD_SET_CONTRAST_B);       //Set contrast for color B
    _sendCmd(0x50);                     //80 (0x50)
    _sendCmd(CMD_SET_CONTRAST_C);       //Set contrast for color C
    _sendCmd(0x7D);                     //125 (0x7D)
    _sendCmd(CMD_MASTER_CURRENT_CONTROL);//master current control
    _sendCmd(0x06);                     //6
    _sendCmd(CMD_SET_PRECHARGE_SPEED_A);//Set Second Pre-change Speed For ColorA
    _sendCmd(0x64);                     //100
    _sendCmd(CMD_SET_PRECHARGE_SPEED_B);//Set Second Pre-change Speed For ColorB
    _sendCmd(0x78);                     //120
    _sendCmd(CMD_SET_PRECHARGE_SPEED_C);//Set Second Pre-change Speed For ColorC
    _sendCmd(0x64);                     //100
    _sendCmd(CMD_SET_REMAP);            //set remap & data format
    _sendCmd(0x72);                     //0x72
    _sendCmd(CMD_SET_DISPLAY_START_LINE);//Set display Start Line
    _sendCmd(0x0);
    _sendCmd(CMD_SET_DISPLAY_OFFSET);   //Set display offset
    _sendCmd(0x0);
    _sendCmd(CMD_NORMAL_DISPLAY);       //Set display mode
    _sendCmd(CMD_SET_MULTIPLEX_RATIO);  //Set multiplex ratio
    _sendCmd(0x3F);
    _sendCmd(CMD_SET_MASTER_CONFIGURE); //Set master configuration
    _sendCmd(0x8E);
    _sendCmd(CMD_POWER_SAVE_MODE);      //Set Power Save Mode
    _sendCmd(0x00);                     //0x00
    _sendCmd(CMD_PHASE_PERIOD_ADJUSTMENT);//phase 1 and 2 period adjustment
    _sendCmd(0x31);                     //0x31
    _sendCmd(CMD_DISPLAY_CLOCK_DIV);    //display clock divider/oscillator frequency
    _sendCmd(0xF0);
    _sendCmd(CMD_SET_PRECHARGE_VOLTAGE);//Set Pre-Change Level
    _sendCmd(0x3A);
    _sendCmd(CMD_SET_V_VOLTAGE);        //Set vcomH
    _sendCmd(0x3E);
    _sendCmd(CMD_DEACTIVE_SCROLLING);   //disable scrolling
    _sendCmd(CMD_NORMAL_BRIGHTNESS_DISPLAY_ON);//set display on
}

void SSD1331_drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if ((x < 0) || (x >= RGB_OLED_WIDTH) || (y < 0) || (y >= RGB_OLED_HEIGHT))
        return;
    //set column point
    _sendCmd(CMD_SET_COLUMN_ADDRESS);
    _sendCmd(x);
    _sendCmd(RGB_OLED_WIDTH-1);
    //set row point
    _sendCmd(CMD_SET_ROW_ADDRESS);
    _sendCmd(y);
    _sendCmd(RGB_OLED_HEIGHT-1);

	GPIO_SetBits(GPIOB, GPIO_Pin_0); //dc
	GPIO_ResetBits(GPIOB, GPIO_Pin_1); //cs

	_sendData(color >> 8);
	_sendData(color);

	GPIO_SetBits(GPIOB, GPIO_Pin_1); //cs
}

void SSD1331_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    if((x0 < 0) || (y0 < 0) || (x1 < 0) || (y1 < 0))
        return;

    if (x0 >= RGB_OLED_WIDTH)  x0 = RGB_OLED_WIDTH - 1;
    if (y0 >= RGB_OLED_HEIGHT) y0 = RGB_OLED_HEIGHT - 1;
    if (x1 >= RGB_OLED_WIDTH)  x1 = RGB_OLED_WIDTH - 1;
    if (y1 >= RGB_OLED_HEIGHT) y1 = RGB_OLED_HEIGHT - 1;

    _sendCmd(CMD_DRAW_LINE);//draw line
    _sendCmd(x0);//start column
    _sendCmd(y0);//start row
    _sendCmd(x1);//end column
    _sendCmd(y1);//end row
    _sendCmd((uint8_t)((color>>11)&0x1F));//R
    _sendCmd((uint8_t)((color>>5)&0x3F));//G
    _sendCmd((uint8_t)(color&0x1F));//B
}

void SSD1331_drawFrame(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t outColor, uint16_t fillColor)
{
    if((x0 < 0) || (y0 < 0) || (x1 < 0) || (y1 < 0))
        return;

    if (x0 >= RGB_OLED_WIDTH)  x0 = RGB_OLED_WIDTH - 1;
    if (y0 >= RGB_OLED_HEIGHT) y0 = RGB_OLED_HEIGHT - 1;
    if (x1 >= RGB_OLED_WIDTH)  x1 = RGB_OLED_WIDTH - 1;
    if (y1 >= RGB_OLED_HEIGHT) y1 = RGB_OLED_HEIGHT - 1;

    _sendCmd(CMD_FILL_WINDOW);//fill window
    _sendCmd(ENABLE_FILL);
    _sendCmd(CMD_DRAW_RECTANGLE);//draw rectangle
    _sendCmd(x0);//start column
    _sendCmd(y0);//start row
    _sendCmd(x1);//end column
    _sendCmd(y1);//end row
    _sendCmd((uint8_t)((outColor>>11)&0x1F));//R
    _sendCmd((uint8_t)((outColor>>5)&0x3F));//G
    _sendCmd((uint8_t)(outColor&0x1F));//B
    _sendCmd((uint8_t)((fillColor>>11)&0x1F));//R
    _sendCmd((uint8_t)((fillColor>>5)&0x3F));//G
    _sendCmd((uint8_t)(fillColor&0x1F));//B
}

void SSD1331_drawCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color) {
	signed char xc = 0;
	signed char yc = 0;
	signed char p = 0;

    // Out of range
    if (x >= RGB_OLED_WIDTH || y >= RGB_OLED_HEIGHT)
        return;

    yc = radius;
    p = 3 - (radius<<1);
    while (xc <= yc)
    {
    	SSD1331_drawPixel(x + xc, y + yc, color);
    	SSD1331_drawPixel(x + xc, y - yc, color);
    	SSD1331_drawPixel(x - xc, y + yc, color);
    	SSD1331_drawPixel(x - xc, y - yc, color);
    	SSD1331_drawPixel(x + yc, y + xc, color);
    	SSD1331_drawPixel(x + yc, y - xc, color);
    	SSD1331_drawPixel(x - yc, y + xc, color);
    	SSD1331_drawPixel(x - yc, y - xc, color);
        if (p < 0) p += (xc++ << 2) + 6;
            else p += ((xc++ - yc--)<<2) + 10;
    }

}

// Set current position in cache
void SSD1331_SetXY(unsigned char x, unsigned char y) {
	CHR_X = x;
	CHR_Y = y;
}

void SSD1331_XY_INK(LcdFontSize size) {
	CHR_X += 6*size;
	if (CHR_X + 6*size > RGB_OLED_WIDTH) {
		CHR_X = 0;
		CHR_Y += 8*size;
		if (CHR_Y + 8*size > RGB_OLED_HEIGHT) {
			CHR_Y = 0;
		}
	}
}

void SSD1331_Chr(LcdFontSize size, unsigned char ch, uint16_t chr_color, uint16_t bg_color) {
	unsigned char y, x, sx, sy;
	uint16_t color;
	/////uint16_t cx=CHR_X*6*size;
	uint16_t cx=CHR_X;
	/////uint16_t cy=CHR_Y*8*size;
	uint16_t cy=CHR_Y;

	if ( (cx + 6*size > RGB_OLED_WIDTH) || (cy + 8*size > RGB_OLED_HEIGHT) ) {
		return;
	}

	// CHR
    if ( (ch >= 0x20) && (ch <= 0x7F) )
    {
        // offset in symbols table ASCII[0x20-0x7F]
        ch -= 32;
    }
    else if (ch >= 0xC0)
    {
        // offset in symbols table CP1251[0xC0-0xFF] (Cyrillic)
        ch -= 96;
    }
    else
    {
        // Ignore unknown symbols
        ch = 95;
    }

    if ((size > FONT_1X) & (ch > 15) & (ch < 26)) {
        ch -= 16;
    	for (sy = 0; sy<size; sy++) {
    	for (y = 0; y<8; y++ ) {
    		//set column point
    		_sendCmd(CMD_SET_COLUMN_ADDRESS);
    		_sendCmd(cx);
    		_sendCmd(RGB_OLED_WIDTH-1);
    		//set row point
    		_sendCmd(CMD_SET_ROW_ADDRESS);
    		_sendCmd(y + cy + sy*8);
    		_sendCmd(RGB_OLED_HEIGHT-1);
    		GPIO_SetBits(GPIOB, GPIO_Pin_0); //dc
    		GPIO_ResetBits(GPIOB, GPIO_Pin_1); //cs
    		for (x = 0; x < 5*size; x++ ) {
    			if ( (((BigNumbers[ch][x+sy*10] >> y) & 0x01 ) & (size == FONT_2X)) |
    				 (((LargeNumbers[ch][x+sy*20] >> y) & 0x01 ) & (size == FONT_4X))

    				) {
    				color = chr_color;
    			}
    			else {
    				color = bg_color;
    			}
				_sendData(color >> 8);
				_sendData(color);
    		}
    	}
    	}
    }
    else {
    	for (y = 0; y<8; y++ ) {
    		for (sy = 0; sy<size; sy++ ) {
    			//set column point
    			_sendCmd(CMD_SET_COLUMN_ADDRESS);
    			_sendCmd(cx);
    			_sendCmd(RGB_OLED_WIDTH-1);
    			//set row point
    			_sendCmd(CMD_SET_ROW_ADDRESS);
    			_sendCmd(y*size + sy + cy);
    			_sendCmd(RGB_OLED_HEIGHT-1);
    			GPIO_SetBits(GPIOB, GPIO_Pin_0); //dc
    			GPIO_ResetBits(GPIOB, GPIO_Pin_1); //cs
    			for (x = 0; x<5; x++ ) {
    				if ((FontLookup[ch][x] >> y) & 0x01) {
    					color = chr_color;
    				}
    				else {
    					color = bg_color;
    				}
    				//SSD1331_drawPixel(x+cx, y+cy, color);
    				for (sx = 0; sx<size; sx++ ) {
    					_sendData(color >> 8);
    					_sendData(color);
    				}
    			}
    			_sendData(bg_color >> 8);
    			_sendData(bg_color);
    			GPIO_SetBits(GPIOB, GPIO_Pin_1); //cs
    		}
    	}
    }

    /////CHR_X++;
    //CHR_X += 6*size;
}

// Print a string to display
void SSD1331_Str(LcdFontSize size, unsigned char dataArray[], uint16_t chr_color, uint16_t bg_color) {
    unsigned char tmpIdx=0;

    while( dataArray[ tmpIdx ] != '\0' )
    {
        /*/////
    	if (CHR_X > 15) {
        	CHR_X = 0;
        	CHR_Y++;
        	if (CHR_Y > 7) {
        		CHR_Y = 0;
        	}
        }
        */
    	/*
    	if (CHR_X + 6*size > RGB_OLED_WIDTH) {
        	CHR_X = 0;
        	CHR_Y += 8*size;
        	if (CHR_Y + 8*size > RGB_OLED_HEIGHT) {
        		CHR_Y = 0;
        	}
        }*/

        SSD1331_Chr(size, dataArray[ tmpIdx ], chr_color, bg_color);
        SSD1331_XY_INK(size);
        tmpIdx++;
    }
}

// Print a string from the Flash to display
void SSD1331_FStr(LcdFontSize size, const unsigned char *dataPtr, uint16_t chr_color, uint16_t bg_color) {
    unsigned char c;
    for (c = *( dataPtr ); c; ++dataPtr, c = *( dataPtr ))
    {
        /*
    	if (CHR_X > 15) {
        	CHR_X = 0;
        	CHR_Y++;
        	if (CHR_Y > 7) {
        		CHR_Y = 0;
        	}
        }
        */

        SSD1331_Chr(size, c, chr_color, bg_color);
        SSD1331_XY_INK(size);
    }
}

void SSD1331_IMG(const unsigned char *img, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	uint16_t xx, yy;

	if ( (x + width > RGB_OLED_WIDTH) | (y+height > RGB_OLED_HEIGHT) ){
		return;
	}

	for (yy=0; yy<height; yy++) {
		//set column point
		_sendCmd(CMD_SET_COLUMN_ADDRESS);
		_sendCmd(x);
		_sendCmd(RGB_OLED_WIDTH-1);
		//set row point
		_sendCmd(CMD_SET_ROW_ADDRESS);
		_sendCmd(y + yy);
		_sendCmd(RGB_OLED_HEIGHT-1);
		GPIO_SetBits(GPIOB, GPIO_Pin_0); //dc
		GPIO_ResetBits(GPIOB, GPIO_Pin_1); //cs

		for (xx=0; xx<width*2; xx++) {
			_sendData(img[yy*width*2 + xx]);
		}
	}
}

void SSD1331_copyWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,uint16_t x2, uint16_t y2)
{
    _sendCmd(CMD_COPY_WINDOW);//copy window
    _sendCmd(x0);//start column
    _sendCmd(y0);//start row
    _sendCmd(x1);//end column
    _sendCmd(y1);//end row
    _sendCmd(x2);//new column
    _sendCmd(y2);//new row
}

void SSD1331_dimWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    _sendCmd(CMD_DIM_WINDOW);//copy area
    _sendCmd(x0);//start column
    _sendCmd(y0);//start row
    _sendCmd(x1);//end column
    _sendCmd(y1);//end row
}

void SSD1331_clearWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    _sendCmd(CMD_CLEAR_WINDOW);//clear window
    _sendCmd(x0);//start column
    _sendCmd(y0);//start row
    _sendCmd(x1);//end column
    _sendCmd(y1);//end row
}

void SSD1331_setScrolling(ScollingDirection direction, uint8_t rowAddr, uint8_t rowNum, uint8_t timeInterval)
{
    uint8_t scolling_horizontal = 0x0;
    uint8_t scolling_vertical = 0x0;
    switch(direction){
        case Horizontal:
            scolling_horizontal = 0x01;
            scolling_vertical = 0x00;
            break;
        case Vertical:
            scolling_horizontal = 0x00;
            scolling_vertical = 0x01;
            break;
        case Diagonal:
            scolling_horizontal = 0x01;
            scolling_vertical = 0x01;
            break;
        default:
            break;
    }
    _sendCmd(CMD_CONTINUOUS_SCROLLING_SETUP);
    _sendCmd(scolling_horizontal);
    _sendCmd(rowAddr);
    _sendCmd(rowNum);
    _sendCmd(scolling_vertical);
    _sendCmd(timeInterval);
    _sendCmd(CMD_ACTIVE_SCROLLING);
}

void SSD1331_enableScrolling(bool enable)
{
    if(enable)
        _sendCmd(CMD_ACTIVE_SCROLLING);
    else
        _sendCmd(CMD_DEACTIVE_SCROLLING);
}

void SSD1331_setDisplayMode(DisplayMode mode)
{
    _sendCmd(mode);
}

void SSD1331_setDisplayPower(DisplayPower power)
{
    _sendCmd(power);
}
