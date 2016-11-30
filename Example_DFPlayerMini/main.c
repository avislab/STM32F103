#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "misc.h"

#include "tim2_delay.h"
#include "mp3_lib.h"

void SetSysClockToHSE(void)
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

        /* Flash 0 wait state */
        //FLASH_SetLatency( FLASH_Latency_0);

        /* HCLK = SYSCLK */
        RCC_HCLKConfig( RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK */
        RCC_PCLK2Config( RCC_HCLK_Div1);

        /* PCLK1 = HCLK */
        RCC_PCLK1Config(RCC_HCLK_Div1);

        /* Select HSE as system clock source */
        RCC_SYSCLKConfig( RCC_SYSCLKSource_HSE);

        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x04)
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
	// Set System clock
	SetSysClockToHSE();

	// Initialize TIM2 for delay functions
	TIM2_init();

    // Initialize USART3 for DFPlayer
	MP3_init();

    // Wait for DFPlayer Initialization
    delay_ms(500);

    // Set DFPlayer Volume
    MP3_send_cmd(MP3_VOLUME, 0, 20); // Volume 0-30
    delay_ms(10);
    // Choose a folder with mp3-files
    // You can use some folders with different languages or different voices
    MP3_set_folder(1);
    delay_ms(10);

    // Play single file from folder
    // This command start playing file 032.mp3 from folder 05
    // MP3_send_cmd(MP3_PLAY_FOLDER_FILE, 5, 32); //folder 01..99, file 001..255

    //Make Voice QUEUE
    MP3_say(MP3_NO_VALUE, 3148, MP3_NO_VALUE);
    //MP3_say(MP3_NO_VALUE, -35, MP3_NO_VALUE);
    //MP3_say(100, 153, 103);
    //MP3_say(MP3_NO_VALUE, 715, MP3_NO_VALUE);

    while (1)
    {
    	// MP3 Voice QUEUE processing
    	MP3_queue_processing();

    	// Here your code
    	// ...
    	delay_ms(100);
    	// ...
    	//
    }
}

