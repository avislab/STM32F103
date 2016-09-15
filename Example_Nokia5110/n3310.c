#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "string.h"
#include "n3310.h"

static void LcdVSPI(unsigned char data);
static void LcdSend(unsigned char data, LcdCmdData cd);

// Display Cache
static unsigned char LcdCache[LCD_CACHE_SIZE];
static int BottomCacheMark;
static int TopCacheMark;
static int LcdCacheIdx;

// Display Initialization
void LcdInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Enable LCD_PORT Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure the GPIO pins */
	GPIO_InitStructure.GPIO_Pin = (LCD_DC_PIN | LCD_CE_PIN | LCD_RST_PIN | LCD_SDIN_PIN | LCD_SCLK_PIN);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LCD_PORT, &GPIO_InitStructure);

	SET_RST;
	RESET_DC;
	RESET_CE;
	RESET_SDIN;
	RESET_SCLK;

    // Reset
	RESET_RST;
	SET_RST;

    // Disable LCD controller
	SET_CE;
  
    // Send commands
    LcdSend(0x21, LCD_CMD); // LCD Extended Commands
    LcdSend(0xC8, LCD_CMD); // Set Contrast (LCD Vop)
    LcdSend(0x06, LCD_CMD); // Set Temperature coefficent
    LcdSend(0x13, LCD_CMD); // LCD bias mode 1:48
    LcdSend(0x20, LCD_CMD); // LCD Standard Commands and Horizontal addressing mode
    LcdSend(0x0C, LCD_CMD); // LCD in normal mode

    // Clear Display
    LcdClear();
    LcdUpdate();
}

// Clear Display
void LcdClear(void)
{
	// Clear cache
    memset(LcdCache, 0x00, LCD_CACHE_SIZE);
    
    // Reset Cache marks
    BottomCacheMark = 0;
    TopCacheMark = LCD_CACHE_SIZE - 1;
}

// Update LCD. Upload Cache to display
void LcdUpdate(void)
{
    int i;

    if (BottomCacheMark < 0)
        BottomCacheMark = 0;
    else if (BottomCacheMark >= LCD_CACHE_SIZE)
        BottomCacheMark = LCD_CACHE_SIZE - 1;

    if (TopCacheMark < 0)
        TopCacheMark = 0;
    else if (TopCacheMark >= LCD_CACHE_SIZE)
        TopCacheMark = LCD_CACHE_SIZE - 1;

    // Set start address
    LcdSend(0x80 | (BottomCacheMark % LCD_X_RES), LCD_CMD);
    LcdSend(0x40 | (BottomCacheMark / LCD_X_RES), LCD_CMD);

    // Refresh display
    for (i = BottomCacheMark; i <= TopCacheMark; i++)
    {
    	LcdSend(LcdCache[i], LCD_DATA);
    }

    // Reset Cache Marks
    BottomCacheMark = LCD_CACHE_SIZE - 1;
    TopCacheMark = 0;
}

// Send data to display (programm SPI)
static void LcdVSPI(unsigned char data) {
  for(unsigned int i = 0; i< 8; i++, data = data << 1){
      if ((data & 0x80) == 0x80) {
    	  SET_SDIN;
      }
      else {
        RESET_SDIN;
      }

      SET_SCLK;
      RESET_SCLK;
  }
}

// Send data or command to display
static void LcdSend(unsigned char data, LcdCmdData cd)
{
  // Enable display controller
	RESET_CE;

  if (cd == LCD_DATA) {
	  SET_DC;
  }
  else {
	  RESET_DC;
  }

  LcdVSPI(data);
    
  // Disable display controller
  SET_CE;
}

// Set contrast 0x00 ... 0x7F
void LcdContrast ( unsigned char contrast )
{
    LcdSend(0x21, LCD_CMD);              // LCD Extended Commands
    LcdSend(0x80 | contrast, LCD_CMD);   // Set contrast
    LcdSend(0x20, LCD_CMD);              // LCD Standard Commands
}

// Set cursor position (standard font size) x = 0...13, y = 0...5
unsigned char LcdGotoXYFont(unsigned char x, unsigned char y)
{
    if(x > 13 || y > 5) return OUT_OF_BORDER;

    LcdCacheIdx = x * 6 + y * 84;
    return OK;
}


// Print a single char into current position
unsigned char LcdChr(LcdFontSize size, unsigned char ch)
{
    unsigned char i, c;
    unsigned char b1, b2;
    int  tmpIdx;

    if (LcdCacheIdx < BottomCacheMark)
    {
        // Set bottom Cache Mark
        BottomCacheMark = LcdCacheIdx;
    }

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
            // Copy symbol image into cache
            LcdCache[LcdCacheIdx++] = *( &(FontLookup[ch][i]) ) << 1;
        }
    }
    else if (size == FONT_2X)
    {
        tmpIdx = LcdCacheIdx - 84;

        if (tmpIdx < BottomCacheMark)
        {
            BottomCacheMark = tmpIdx;
        }

        if (tmpIdx < 0) return OUT_OF_BORDER;

        // The Big numbers defined in BigNumbers array
        // Other symbols will scale
        if ((ch > 15) & (ch < 26)) {
          ch -= 16;
          for (i = 0; i < 10; i++)
          {
            LcdCache[tmpIdx++] = *(&(BigNumbers[ch][i]));
            LcdCache[tmpIdx+83] = *(&(BigNumbers[ch][10+i]));
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
              LcdCache[tmpIdx++] = b1;
              LcdCache[tmpIdx++] = b1;
              LcdCache[tmpIdx + 82] = b2;
              LcdCache[tmpIdx + 83] = b2;
          }
        }

        // Set new cursor position
        LcdCacheIdx = (LcdCacheIdx + 11) % LCD_CACHE_SIZE;
    }
    else if (size == FONT_4X) {
        tmpIdx = LcdCacheIdx - 84;

        if (tmpIdx < BottomCacheMark)
        {
            BottomCacheMark = tmpIdx;
        }

        if (tmpIdx < 0) return OUT_OF_BORDER;
        
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
            LcdCache[tmpIdx++] = *(&(LargeNumbers[ch][i]));
            LcdCache[tmpIdx+83] = *(&(LargeNumbers[ch][20+i]));
            LcdCache[tmpIdx+167] = *(&(LargeNumbers[ch][40+i]));
            LcdCache[tmpIdx+251] = *(&(LargeNumbers[ch][60+i]));
          }
        }

        // Set new cursor position
        LcdCacheIdx = (LcdCacheIdx + 20) % LCD_CACHE_SIZE;
        
        if (ch == 12) { // .
          LcdCacheIdx -=5;
        }
    }

    if (LcdCacheIdx > TopCacheMark)
    {
        // Set top Cache Mark
        TopCacheMark = LcdCacheIdx;
    }

    // Horizontal space between chars
    LcdCache[LcdCacheIdx] = 0x00;

    if(LcdCacheIdx == (LCD_CACHE_SIZE - 1))
    {
        LcdCacheIdx = 0;
        return OK_WITH_WRAP;
    }

    LcdCacheIdx++;
    return OK;
}

// Print a string to display
unsigned char LcdStr(LcdFontSize size, unsigned char dataArray[])
{
    unsigned char tmpIdx=0;
    unsigned char response;
    while( dataArray[ tmpIdx ] != '\0' )
    {
        response = LcdChr(size, dataArray[ tmpIdx ]);
        if( response == OUT_OF_BORDER)
            return OUT_OF_BORDER;
        tmpIdx++;
    }
    return OK;
}

// Print a string from the Flash to display
unsigned char LcdFStr(LcdFontSize size, const unsigned char *dataPtr)
{
    unsigned char c;
    unsigned char response;
    for (c = *( dataPtr ); c; ++dataPtr, c = *( dataPtr ))
    {
        response = LcdChr( size, c );
        if(response == OUT_OF_BORDER)
            return OUT_OF_BORDER;
    }
    return OK;
}

// Draw pixel
unsigned char LcdPixel(unsigned char x, unsigned char y, LcdPixelMode mode)
{
    int  index;
    unsigned char  offset;
    unsigned char  data;

    if (x >= LCD_X_RES || y >= LCD_Y_RES) return OUT_OF_BORDER;

    index = ( ( y / 8 ) * 84 ) + x;
    offset  = y - ( ( y / 8 ) * 8 );

    data = LcdCache[ index ];

    // PIXEL_OFF
    if (mode == PIXEL_OFF)
    {
        data &= ( ~( 0x01 << offset ) );
    }
    // PIXEL_ON
    else if (mode == PIXEL_ON)
    {
        data |= ( 0x01 << offset );
    }
    // PIXEL_XOR
    else if (mode  == PIXEL_XOR)
    {
        data ^= ( 0x01 << offset );
    }

    // Copy result to the cache
    LcdCache[index] = data;

    if (index < BottomCacheMark)
    {
        // Set new bottom Cache Mark
        BottomCacheMark = index;
    }

    if (index > TopCacheMark)
    {
    	// Set new top Cache Mark
        TopCacheMark = index;
    }
    return OK;
}

// Draw line
unsigned char LcdLine(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, LcdPixelMode mode)
{
    int dx, dy, stepx, stepy, fraction;
    unsigned char response;

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
    response = LcdPixel(x1, y1, mode);
    if (response)
        return response;

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

            response = LcdPixel(x1, y1, mode);
            if(response)
                return response;

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

            response = LcdPixel(x1, y1, mode);
            if (response)
                return response;
        }
    }

    return OK;
}

// Draw Circle
unsigned char LcdCircle(unsigned char x, unsigned char y, unsigned char radius, LcdPixelMode mode)
{
    signed char xc = 0;
    signed char yc = 0;
    signed char p = 0;

    if (x >= LCD_X_RES || y >= LCD_Y_RES) return OUT_OF_BORDER;

    yc = radius;
    p = 3 - (radius<<1);
    while (xc <= yc)  
    {
        LcdPixel(x + xc, y + yc, mode);
        LcdPixel(x + xc, y - yc, mode);
        LcdPixel(x - xc, y + yc, mode);
        LcdPixel(x - xc, y - yc, mode);
        LcdPixel(x + yc, y + xc, mode);
        LcdPixel(x + yc, y - xc, mode);
        LcdPixel(x - yc, y + xc, mode);
        LcdPixel(x - yc, y - xc, mode);
        if (p < 0) p += (xc++ << 2) + 6;
            else p += ((xc++ - yc--)<<2) + 10;
    }

    return OK;
}

// Draw Rect
unsigned char LcdRect(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, LcdPixelMode mode)
{
    unsigned char tmpIdx;

    // Return if border offsets
    if ((x1 >= LCD_X_RES) ||  (x2 >= LCD_X_RES) || (y1 >= LCD_Y_RES) || (y2 >= LCD_Y_RES))
        return OUT_OF_BORDER;

    if ((x2 > x1) && (y2 > y1))
    {
        // Draw horizontal lines
        for (tmpIdx = x1; tmpIdx <= x2; tmpIdx++)
        {
            LcdPixel(tmpIdx, y1, mode);
            LcdPixel(tmpIdx, y2, mode);
        }

        // Draw vertical lines
        for (tmpIdx = y1; tmpIdx <= y2; tmpIdx++)
        {
            LcdPixel(x1, tmpIdx, mode);
            LcdPixel(x2, tmpIdx, mode);
        }
    }
    return OK;
}

// Draw Image
void LcdImage(const unsigned char *imageData)
{
    // Copy data to the cache
    memcpy(LcdCache, imageData, LCD_CACHE_SIZE);
    
    BottomCacheMark = 0;
    TopCacheMark = LCD_CACHE_SIZE - 1;
}

// Copy data to cache
void LcdWriteToCache(int addr, unsigned char data) {
  LcdCache[addr] = data;
}

// Invert string line
void LcdIvertLine(unsigned char line) {
  unsigned char x;
  unsigned int addr;
  
  addr=line*LCD_X_RES;
  for (x=0; x<LCD_X_RES; x++) {
    LcdCache[addr] = ~LcdCache[addr];
    addr++;
  }
}

// Invert fragment of string line
void LcdIvertLineFragment(unsigned char line, unsigned char chr_x1, unsigned char chr_x2) {
  unsigned int addr, addr_start, addr_end;
  addr_start = line*LCD_X_RES + chr_x1*6;
  addr_end = line*LCD_X_RES + chr_x2*6;

  for (addr=addr_start; addr<addr_end; addr++) {
    LcdCache[addr] = ~LcdCache[addr];
  }
}
