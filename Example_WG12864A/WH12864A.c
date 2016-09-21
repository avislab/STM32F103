#include "string.h"
#include "WG12864A.h"

#define NOP() asm("nop")

static unsigned char LCDCache[8][128];
static unsigned char LCDLeftCacheMark = 0;
static unsigned char LCDRightCacheMark = 0;
static unsigned char LCDTopCacheMark = 0;
static unsigned char LCDBottomCacheMark = 0;
static unsigned char LCD_X, LCD_Y, LCD_CS;

void delay_us(uint32_t n_usec);

// Send command to LCD
void LCD_WriteCom(unsigned char Com,unsigned char CS)
{
	#ifdef WG12864B
		GPIO_ResetBits(LCD_COM_PORT, CS);
	#else
		GPIO_SetBits(LCD_COM_PORT, CS);
	#endif

	LCD_DI_OFF;
	LCD_RW_OFF;
	NOP();
	NOP();
	GPIO_Write(LCD_DATA_PORT, Com);
	LCD_E_ON;
	NOP();
	NOP();
	LCD_E_OFF;
	delay_us(5);

	#ifdef WG12864B
		GPIO_SetBits(LCD_COM_PORT, LCD_CS1 | LCD_CS2);
	#else
		GPIO_ResetBits(LCD_COM_PORT, LCD_CS1 | LCD_CS2);
	#endif
}

// Send data to LCD
void LCD_WriteData(unsigned char data,unsigned char CS)
{
	#ifdef WG12864B
		GPIO_ResetBits(LCD_COM_PORT, CS);
	#else
		GPIO_SetBits(LCD_COM_PORT, CS);
	#endif

	LCD_DI_ON;
	LCD_RW_OFF;
	NOP();
	NOP();
	GPIO_Write(LCD_DATA_PORT, data);
	LCD_E_ON;
	NOP();
	NOP();
	LCD_E_OFF;
	delay_us(5);

	#ifdef WG12864B
		GPIO_SetBits(LCD_COM_PORT, LCD_CS1 | LCD_CS2);
	#else
		GPIO_ResetBits(LCD_COM_PORT, LCD_CS1 | LCD_CS2);
	#endif
}

// Set current position
void LCD_WriteXY(unsigned char x,unsigned char y,const unsigned char CS)
{
	LCD_WriteCom(0xb8+y,CS);
	LCD_WriteCom(0x40+x,CS);
}

// Mark changed positions in the cache
void LCD_Cache_Mark(unsigned char x, unsigned char yy) {
	if (x > LCD_X_RES -1) x = LCD_X_RES -1;
	if (yy > LCD_Y_RES/8 -1) x = LCD_Y_RES/8 -1;

	if (x < LCDLeftCacheMark) LCDLeftCacheMark = x;
	if (x > LCDRightCacheMark) LCDRightCacheMark = x;

	if (yy < LCDTopCacheMark) LCDTopCacheMark = yy;
	if (yy > LCDBottomCacheMark) LCDBottomCacheMark = yy;
}

// Set current position in cache
void LCD_SetXY(unsigned char x, unsigned char yy) {
	LCD_X = x;
	LCD_Y = yy;

	if(x > 63) {
		x = x-64;
		LCD_CS = LCD_CS2;
	}
	else {
		LCD_CS = LCD_CS1;
	}

	LCD_WriteXY(x, yy, LCD_CS);
}

// Increment position X
void LCD_X_INC(void) {
	LCD_X++;

	if (LCD_X >= LCD_X_RES) {
		LCD_X = 0;
		LCD_Y++;
		if (LCD_Y >= LCD_Y_RES) {
			LCD_Y = 0;
		}
		LCD_SetXY(LCD_X, LCD_Y);
	}
}

// ===========================================================
// LCD initialization
void LcdInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* Initialize LCD */
	// Enable PORTA and PORTB Clock
	RCC_APB2PeriphClockCmd(LCD_ENABLE_PORTS_CLOCK, ENABLE);

	/* Configure the GPIO pins */
	GPIO_InitStructure.GPIO_Pin = LCD_DATA_PINS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LCD_DATA_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = LCD_RST | LCD_E | LCD_RW | LCD_DI | LCD_CS1 | LCD_CS2;
	GPIO_Init(LCD_COM_PORT, &GPIO_InitStructure);

	LCD_RST_ON;
	delay_us(5);

	LCD_WriteXY(0,0,(LCD_CS1 | LCD_CS2));
	LCD_WriteCom(0xc0,(LCD_CS1 | LCD_CS2));
	LCD_WriteCom(0x3f,(LCD_CS1 | LCD_CS2));
}

// Clear LCD cache
void LcdClear(void)
{
	unsigned char x,y;
	for(x=0;x<64;x++)
	{
		for(y=0;y<8;y++)
		{
			LCD_WriteXY(x,y,(LCD_CS1 | LCD_CS2));
			LCD_WriteData(0,(LCD_CS1 | LCD_CS2));
			// Clear Cache
			LCDCache[y][x] = 0;
			LCDCache[y][x+64] = 0;
		}
	}
}

// Upload cache to LCD
void LcdUpdate (void) {
	unsigned char x, yy;
	unsigned char CS_FLAG = 0;

	for (yy=LCDTopCacheMark; yy<=LCDBottomCacheMark; yy++) {
		LCD_SetXY(LCDLeftCacheMark, yy);
		for (x=LCDLeftCacheMark; x<=LCDRightCacheMark; x++){
 			if ((x > 63) && (CS_FLAG == 0)) {
 				CS_FLAG = 1;
 				LCD_SetXY(x, yy);
 			}
			LCD_WriteData(LCDCache[yy][x], LCD_CS);
		}
		CS_FLAG = 0;
	}
	LCDLeftCacheMark = 0;
	LCDRightCacheMark = 0;
	LCDTopCacheMark = 0;
	LCDBottomCacheMark = 0;
}

// Draw Pixel
void LcdPixel (unsigned char x, unsigned char y, const unsigned char color) {
	unsigned char temp=0;
	uint8_t yy;

	// Out of range
	if ((x >= LCD_X_RES) || (y >= LCD_Y_RES))
		return;

	yy = y/8;
	temp = LCDCache[yy][x];
	if (color==1) temp |= (1<<(y-yy*8));	//Enable/Disable bit
	else temp &=~(1<<(y-yy*8));
	LCDCache[yy][x] = temp;
	LCD_Cache_Mark(x, yy);
}

// Draw line
void LcdLine(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, const unsigned char color) {
    int dx, dy, stepx, stepy, fraction;

    // dy   y2 - y1
    // -- = -------
    // dx   x2 - x1

    dy = y2 - y1;
    dx = x2 - x1;

    if (dy < 0)
    {
        dy    = -dy;
        stepy = -1;
    }
    else
    {
        stepy = 1;
    }

    if (dx < 0)
    {
        dx    = -dx;
        stepx = -1;
    }
    else
    {
        stepx = 1;
    }

    dx <<= 1;
    dy <<= 1;

    // Draw start point
    LcdPixel(x1, y1, color);

    // Draw next points
    if (dx > dy)
    {
        fraction = dy - ( dx >> 1);
        while (x1 != x2)
        {
            if (fraction >= 0)
            {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;

            LcdPixel(x1, y1, color);
        }
    }
    else
    {
        fraction = dx - ( dy >> 1);
        while (y1 != y2)
        {
            if (fraction >= 0)
            {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;

            LcdPixel(x1, y1, color);
        }
    }
}

// Draw Circle
void LcdCircle(unsigned char x, unsigned char y, unsigned char radius,  const unsigned char color) {
    signed char xc = 0;
    signed char yc = 0;
    signed char p = 0;

    // Out of range
    if (x >= LCD_X_RES || y >= LCD_Y_RES)
    	return;

    yc = radius;
    p = 3 - (radius<<1);
    while (xc <= yc)
    {
        LcdPixel(x + xc, y + yc, color);
        LcdPixel(x + xc, y - yc, color);
        LcdPixel(x - xc, y + yc, color);
        LcdPixel(x - xc, y - yc, color);
        LcdPixel(x + yc, y + xc, color);
        LcdPixel(x + yc, y - xc, color);
        LcdPixel(x - yc, y + xc, color);
        LcdPixel(x - yc, y - xc, color);
        if (p < 0) p += (xc++ << 2) + 6;
            else p += ((xc++ - yc--)<<2) + 10;
    }
}

// Draw Rect
void LcdRect(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2,  const unsigned char color) {
    unsigned char tmpIdx;

    // Out of range
    if ((x1 >= LCD_X_RES) ||  (x2 >= LCD_X_RES) || (y1 >= LCD_Y_RES) || (y2 >= LCD_Y_RES))
        return;

    if ((x2 > x1) && (y2 > y1))
    {
        // Draw horizontal lines
        for (tmpIdx = x1; tmpIdx <= x2; tmpIdx++)
        {
            LcdPixel(tmpIdx, y1, color);
            LcdPixel(tmpIdx, y2, color);
        }

        // Draw vertical lines
        for (tmpIdx = y1; tmpIdx <= y2; tmpIdx++)
        {
            LcdPixel(x1, tmpIdx, color);
            LcdPixel(x2, tmpIdx, color);
        }
    }
}

// Set cursor position (standard font size) x = 0...20, y = 0...7
void LcdGotoXYFont(unsigned char x, unsigned char y) {
    LCD_X = x*6;
    LCD_Y = y;

    if (LCD_X >= LCD_X_RES) LCD_X = 0;
    if (LCD_Y >= LCD_Y_RES/8) LCD_Y = 0;
}


// Print a single char into current position
void LcdChr(LcdFontSize size, unsigned char ch) {
    unsigned char i, c;
    unsigned char b1, b2;

    // Out of range
    if ( (LCD_X + 6*size >= LCD_X_RES) || (LCD_Y + (size-1) > LCD_Y_RES/8 -1) )
    	return;


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

    if (size == FONT_1X)
    {
        for (i = 0; i < 5; i++)
        {
			LCD_Cache_Mark(LCD_X, LCD_Y);
            // Copy symbol image into cache
            LCDCache[LCD_Y][LCD_X++] = *( &(FontLookup[ch][i]) ) << 1;
            LCD_Cache_Mark(LCD_X, LCD_Y);
        }
    }
    else if (size == FONT_2X)
    {

        // The Big numbers defined in BigNumbers array
        // Other symbols will scale
        if ((ch > 15) & (ch < 26)) {
          ch -= 16;
          for (i = 0; i < 10; i++)
          {
            LCDCache[LCD_Y][LCD_X] = *(&(BigNumbers[ch][i]));
            LCDCache[LCD_Y+1][LCD_X] = *(&(BigNumbers[ch][10+i]));
			LCD_Cache_Mark(LCD_X, LCD_Y);
            //LCD_X++;
            LCD_X_INC();
            LCD_Cache_Mark(LCD_X+1, LCD_Y+1);
          }
        }
        else{
          for (i = 0; i < 5; i++)
          {
              // Copy symbol image from the table to the temporart variable
              c = *(&(FontLookup[ch][i])) << 1;
              // Scale image
              // First part
              b1 =  (c & 0x01) * 3;
              b1 |= (c & 0x02) * 6;
              b1 |= (c & 0x04) * 12;
              b1 |= (c & 0x08) * 24;

              c >>= 4;
              // Second part
              b2 =  (c & 0x01) * 3;
              b2 |= (c & 0x02) * 6;
              b2 |= (c & 0x04) * 12;
              b2 |= (c & 0x08) * 24;

              // Copy to the cache
              LCDCache[LCD_Y][LCD_X] = b1;
              LCDCache[LCD_Y][LCD_X+1] = b1;
              LCDCache[LCD_Y+1][LCD_X] = b2;
              LCDCache[LCD_Y+1][LCD_X+1] = b2;
              LCD_Cache_Mark(LCD_X, LCD_Y);
			  LCD_X_INC();
			  LCD_X_INC();
			  LCD_Cache_Mark(LCD_X, LCD_Y+1);
          }
        }
    }
    else if (size == FONT_4X) {

        // The LARGE numbers defined in LargeNumbers array
        // Other symbols will ignored
        // "+", "-" and "."
        if ((ch > 15) & (ch < 26)) {
          ch -= 16;
        }
        else if (ch == 43-32) { // +
          ch = 10;
        }
        else if (ch == 45-32) { // -
          ch = 11;
        }
        else if (ch == 46-32) { // .
          ch = 12;
        }
        else {
          ch= 255;
        }

        if (ch != 255) {
          for (i = 0; i < 20; i++)
          {
            LCDCache[LCD_Y][LCD_X] = *(&(LargeNumbers[ch][i]));
            LCDCache[LCD_Y+1][LCD_X] = *(&(LargeNumbers[ch][20+i]));
            LCDCache[LCD_Y+2][LCD_X] = *(&(LargeNumbers[ch][40+i]));
            LCDCache[LCD_Y+3][LCD_X] = *(&(LargeNumbers[ch][60+i]));
			LCD_Cache_Mark(LCD_X, LCD_Y);
            LCD_X_INC();
            LCD_Cache_Mark(LCD_X, LCD_Y+3);
          }
        }

        if (ch == 12) { // .
          LCD_X -=5;
        }
    }

    // Horizontal space between chars
    LCDCache[LCD_Y][LCD_X] = 0x00;
    LCD_X_INC();
    LCD_Cache_Mark(LCD_X, LCD_Y);

}

// Print a string to display
void LcdStr(LcdFontSize size, unsigned char dataArray[]) {
    unsigned char tmpIdx=0;

    while( dataArray[ tmpIdx ] != '\0' )
    {
        if (LCD_X > LCD_X_RES - 5) {
        	LCD_X = 0;
        	LCD_Y++;
        	if (LCD_Y > 7) {
        		LCD_Y = 0;
        	}
        	LCD_Cache_Mark(LCD_X, LCD_Y);
        }
        LcdChr(size, dataArray[ tmpIdx ]);
        tmpIdx++;
    }
}


// Print a string from the Flash to display
void LcdFStr(LcdFontSize size, const unsigned char *dataPtr) {
    unsigned char c;
    for (c = *( dataPtr ); c; ++dataPtr, c = *( dataPtr ))
    {
        if (LCD_X > LCD_X_RES - 5) {
        	LCD_X = 0;
        	LCD_Y++;
        	if (LCD_Y > 7) {
        		LCD_Y = 0;
        	}
        	LCD_Cache_Mark(LCD_X, LCD_Y);
        }
        LcdChr(size, c);
    }
}

void LcdWriteToCache(unsigned char x, unsigned char y, unsigned char data) {
	LCDCache[y][x] = data;
    LCD_Cache_Mark(x, y);
}

void LcdIvertLineFragment(unsigned char line, unsigned char chr_x1, unsigned char chr_x2) {
	unsigned char x;
	chr_x1 = chr_x1 * 6;
	chr_x2 = chr_x2 * 6;

	if (chr_x1 > LCD_X_RES-1) chr_x1 = LCD_X_RES-1;
	if (chr_x2 > LCD_X_RES-1) chr_x2 = LCD_X_RES-1;

	for (x=chr_x1; x<=chr_x2; x++) {
		LCDCache[line][x] = ~(LCDCache[line][x]);
	}
	LCD_SetXY(chr_x1, line);
	LCD_SetXY(chr_x2, line);
}

void LcdIvertLine(unsigned char line) {
	unsigned char x;

	for (x=0; x<=127; x++) {
		LCDCache[line][x] = ~(LCDCache[line][x]);
	}
	LCD_SetXY(0, line);
	LCD_SetXY(127, line);
}


// Draw Image
void LcdImage(const unsigned char *imageData) {
	// Copy data to the cache
	memcpy(LCDCache, imageData, LCD_CACHE_SIZE);

	LCDLeftCacheMark = 0;
	LCDRightCacheMark = 127;
	LCDTopCacheMark = 0;
	LCDBottomCacheMark = 7;
}
