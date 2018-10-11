#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "misc.h"

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

void TIM4_IRQHandler(void)
{
        if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
        {
        	// Îáîâ'ÿçêîâî ñêèäàºìî ïðàïîð. ßêùî öüîãî íå çðîáèòè, ï³ñëÿ îáðîáêè ïåðåðèâàííÿ çíîâó ïîïàäåìî ñþäè
        	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
			GPIOC->ODR ^= GPIO_Pin_13;
        }
}

int main(void)
{
	SetSysClockToHSE();

	/* Initialize LED which connected to PC13 */
	GPIO_InitTypeDef  GPIO_InitStructure;
	// Enable PORTC Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOC, GPIO_Pin_13); // Set C13 to Low level ("0")

    // TIMER4
    TIM_TimeBaseInitTypeDef TIMER_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); // Âìèêàºìî òàêòóâàííÿ òàéìåðà TIM4

  	TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up; // Ðåæèì ðàõóíêó
    TIMER_InitStructure.TIM_Prescaler = 8000; // Ïîä³ëþâà÷ ÷àñòîòè äëÿ òàéìåðà
    // Òðåáà ùå âðàõîâóâàòè ÿê íàëàøòîâàí³ ïîä³ëþâà÷³ RCC_HCLKConfig( RCC_SYSCLK_Div1); RCC_PCLK1Config(RCC_HCLK_Div1);
    // Ó íàøîìó âèïàäêó îáèäâà  = RCC_SYSCLK_Div1, òîáòî äî ïîä³ëþâà÷à òàéìåðà äîõîäèòü ÷àñòîòà çîâí³øíüîãî êâàðöó (8ÌÃö)
    TIMER_InitStructure.TIM_Period = 500; // Ïåð³îä, ÷åðåç ÿêèé âèêîíóºòüñÿ ïåðåðèâàííÿ ïî ïåðåïîâíåííþ  // F=8000000/8000/500 = 2 ðàçè/ñåê.
    TIM_TimeBaseInit(TIM4, &TIMER_InitStructure);
	
    /* 
        This next line needs to be put here!!!
        That's why the TIM_TimeBaseInit(TIM2, &TIMER_InitStructure); line sets the UIF flag
        We are clearing it!!!
    */
        
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); // Âìèêàºìî ïåðåðèâàííÿ ïî ïåðåïîâíåííþ òàéìåðà
    TIM_Cmd(TIM4, ENABLE);// Âìèêàºìî òàéìåð

    /* NVIC Configuration */
    /* Enable the TIM4_IRQn Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    while(1)
    {
    	// Ó ãîëîâíîìó öèêë³ ðîáèìî ùî íàì çàìàíåòüñÿ.
    }
}
