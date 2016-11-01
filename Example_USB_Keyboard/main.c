#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "keycodes.h"

/* Private variables ---------------------------------------------------------*/
__IO uint8_t PrevXferComplete = 1;

extern uint8_t key_buf[9];

void MOUSE_move(int8_t x, int8_t y){
	/*
	 * buf[0]: 1 - report ID
	 * buf[1]: bit2 - middle button, bit1 - right, bit0 - left
	 * buf[2]: move X
	 * buf[3]: move Y
	 * buf[4]: wheel
	 */
	uint8_t buf[5] = {1,0,0,0,0};
	buf[2] = x; buf[3] = y;
	USB_SIL_Write(EP1_IN, buf, 5);
	PrevXferComplete = 0;
	SetEPTxValid(ENDP1);
}

void KEYBOARD_SEND_key_buf(void) {
    USB_SIL_Write(EP1_IN, key_buf, 9);
    PrevXferComplete = 0;
    SetEPTxValid(ENDP1);
    while (PrevXferComplete == 0)
    {}
}

void KEYBOARD_SEND_Char(char ch) {
    press_key(ch);
    KEYBOARD_SEND_key_buf();
    release_key();
    KEYBOARD_SEND_key_buf();
}

void KEYBOARD_SEND_word(char *wrd){
	do {
		KEYBOARD_SEND_Char(*wrd);
	} while(*(++wrd));
}

void SetSysClockTo72(void)
{
	ErrorStatus HSEStartUpStatus;
    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig( RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
    	//FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable);

        /* Flash 2 wait state */
        //FLASH_SetLatency( FLASH_Latency_2);

        /* HCLK = SYSCLK */
        RCC_HCLKConfig( RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK */
        RCC_PCLK2Config( RCC_HCLK_Div1);

        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config( RCC_HCLK_Div2);

        /* PLLCLK = 8MHz * 9 = 72 MHz */
        RCC_PLLConfig(0x00010000, RCC_PLLMul_9);

        /* Enable PLL */
        RCC_PLLCmd( ENABLE);

        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
    else
    { /* If HSE fails to start-up, the application will have wrong clock configuration.
     User can add here some code to deal with this error */

        /* Go to infinite loop */
        while (1)
        {
        }
    }
}

int main(void)
{
  Set_System();
  SetSysClockTo72();
  USB_Interrupts_Config();
  Set_USBClock();
  USB_Init();

  while (1)
  {

    if (bDeviceState == CONFIGURED)
    {
      if (PrevXferComplete)
      {
    	  KEYBOARD_SEND_word("HELLO!!!");
    	  MOUSE_move(1,-1);
    	  //RHIDCheckState();
      }
    }
  }
}

