#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "stdio.h"
#include "misc.h"

volatile int TimeResult;
volatile int TimeSec;
volatile uint8_t TimeState = 0;

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


void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		// Обов'язково скидаємо прапор. Якщо цього не зробити, п_сля обробки переривання знову попадемо сюди
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		TimeSec++;
	}
}

void usart_init(void)
{
	/* Enable USART1 and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	/* Configure the GPIOs */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure the USART1 */
	USART_InitTypeDef USART_InitStructure;

	/* USART1 configuration ------------------------------------------------------*/
	/* USART1 configured as follow:
	          - BaudRate = 115200 baud
	          - Word Length = 8 Bits
	          - One Stop Bit
	          - No parity
	          - Hardware flow control disabled (RTS and CTS signals)
	          - Receive and transmit enabled
	          - USART Clock disabled
	          - USART CPOL: Clock is active low
	          - USART CPHA: Data is captured on the middle
	          - USART LastBit: The clock pulse of the last data bit is not output to
	                           the SCLK pin
	    */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART1 */
	USART_Cmd(USART1, ENABLE);
}

void USARTSend(char *pucBuffer)
{
	while (*pucBuffer)
    {
        USART_SendData(USART1, *pucBuffer++);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {
        }
    }
}

int main(void)
{
	char buffer[80] = {'\0'};
	SetSysClockTo72();

	/* Initialize LED which connected to PC13 */
	GPIO_InitTypeDef  GPIO_InitStructure;
	// Enable PORTC Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_13); // Set C13 to Low level ("0")

	/* Initialize Button input PB0 PB1 */
	// Enable PORTB Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	/* Configure the GPIO_BUTTON pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

    // TIMER4
    TIM_TimeBaseInitTypeDef TIMER_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); // Вмикаємо тактування таймера TIM4

  	TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up; // Режим рахунку
    TIMER_InitStructure.TIM_Prescaler = 7200; // Поділювач таймера
    TIMER_InitStructure.TIM_Period = 10000; // Період, через який виконується переривання по переповненню  // F=72000000/7200/10000 = 1 раз/сек.
    TIM_TimeBaseInit(TIM4, &TIMER_InitStructure);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); // Вмикаємо переривання по переповнення таймера
    TIM_Cmd(TIM4, ENABLE);// Вмикаємо таймер

    /* NVIC Configuration */
    /* Enable the TIM4_IRQn Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    usart_init();

    while(1)
    {
    	if (TimeState == 0) {
    		// Кнопка запускає відлік таймера
    		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0) {
    			TIM_Cmd(TIM4, ENABLE);
    			TIM_SetCounter(TIM4, 0);
    			TimeSec = 0;
    			// Set Status "ON"
    			TimeState = 1;

    			// OFF LED
    			GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    			USARTSend("Started...");
    		}
    	}

    	if (TimeState == 1) {
    		// Кнопка зупиняє відлік таймера
    		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0) {
    			TimeResult = TIM_GetCounter(TIM4)/10 + TimeSec * 1000; // Time im msec
    			TIM_Cmd(TIM4, DISABLE);
    			TimeState = 0;

    			// ON LED
    			GPIO_SetBits(GPIOC, GPIO_Pin_13);

        		sprintf(buffer, "Time: %d ms\r\n", TimeResult);
        		USARTSend(buffer);
    		}
    	}
    }
}
