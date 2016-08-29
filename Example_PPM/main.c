#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

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


volatile uint16_t PPMBuffer[] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
volatile uint8_t PPMi = 0;
volatile uint16_t PPMValue_Prev, PPMValue;

void ppm_init() {

	GPIO_InitTypeDef gpio_cfg;
	GPIO_StructInit(&gpio_cfg);

	/* Timer TIM2, channel 1 */
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	  //gpio_cfg.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	  gpio_cfg.GPIO_Mode = GPIO_Mode_IPU;
	  gpio_cfg.GPIO_Pin = GPIO_Pin_1;
	  GPIO_Init(GPIOA, &gpio_cfg);

	  /* Timer TIM2 enable clock */
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	  /* Timer TIM2 settings */
	  TIM_TimeBaseInitTypeDef timer_base;
	  TIM_TimeBaseStructInit(&timer_base);
	  timer_base.TIM_Prescaler = 72;
	  TIM_TimeBaseInit(TIM2, &timer_base);

	  /* Signal capture settings:
	   - Channel: 1
	   - Count: Up
	   - Source: Input
	   - Divider: Disable
	   - Filter: Disable */
	  TIM_ICInitTypeDef timer_ic;
	  timer_ic.TIM_Channel = TIM_Channel_2;
	  //timer_ic.TIM_ICPolarity = TIM_ICPolarity_BothEdge; # !!! BothEdge not supported
	  timer_ic.TIM_ICPolarity = TIM_ICPolarity_Rising;
	  timer_ic.TIM_ICSelection = TIM_ICSelection_DirectTI;
	  timer_ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	  timer_ic.TIM_ICFilter = 0;
	  TIM_ICInit(TIM2, &timer_ic);

	  /* Enable Interrupt by overflow */
	  TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);
	  /* Timer TIM2 Enable */
	  TIM_Cmd(TIM2, ENABLE);
	  /* Enable Interrupt of Timer TIM2 */
	  NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void){
	volatile uint16_t PPM;

	if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)
	  {
	    /* Reset flag */
	    TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);

	    //USART_SendData(USART1, '!');

	    PPMValue_Prev = PPMValue;
	    PPMValue = TIM_GetCapture2(TIM2);
	    PPM = (PPMValue >= PPMValue_Prev) ? (PPMValue - PPMValue_Prev) : (UINT16_MAX - PPMValue_Prev + PPMValue);
	    if (PPM < 3000) { // it was before 10000
	    	PPMBuffer[PPMi] = PPM;
	    	PPMi++;
		    if (PPMi > 7) {
		    	PPMi = 0;
		    }
	    }
	    else {
	    	PPMi = 0;
	    }

	    /* over-capture */
	    if (TIM_GetFlagStatus(TIM2, TIM_FLAG_CC2OF) != RESET)
	    {
	      TIM_ClearFlag(TIM2, TIM_FLAG_CC2OF);
	      // ...
	    }
	  }
}

int main(void)
{
	SetSysClockTo72();
	ppm_init();

    while(1)
    {
    	// You can read PPM data from array PPMBuffer
    	// For example: PPMBuffer[n]; n - number of channel 0..7
    }
}
