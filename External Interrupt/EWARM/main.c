/*****************************************************************************************************/

/* Includes libraries */
#include <misc.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"

/*****************************************************************************************************/

/* Variables */
uint16_t uiLedTm = 0;
uint16_t enLedState = 0u;
int count = 0;
/* Debounce variables */
uint8_t ucButtonDebCnt = 4;
uint8_t uiInDebTm = 0;
uint8_t ucButtonPrevValue = 1;
uint8_t ucButtonValue = 1;

/*****************************************************************************************************/

/* Control function */

void fnvLedCtrl(void)
{
    if (uiLedTm == 0)
    {
        switch (enLedState)
        {
            /*IDLE - Sempre desligado.*/
            case 0:
            {
                //LED desligado
		GPIO_ResetBits(GPIOC, GPIO_Pin_13); // Set C13 to Low level ("0")
		if (ucButtonValue == 0)
		{
		    enLedState = 1;
		}
            }
            break;
			
            case 1:
            {
                //LED ligado
		//GPIO_SetBits(GPIOC, GPIO_Pin_13); // Set C13 to High level ("1")
              /* do something */
                GPIOC->ODR ^= GPIO_Pin_13;
                for (int i=0; i<1000000; i++){
                }
                GPIOC->ODR ^= GPIO_Pin_13; 
                for (int i=0; i<1000000; i++){
                }
                GPIOC->ODR ^= GPIO_Pin_13;
                for (int i=0; i<1000000; i++){
                }
                GPIOC->ODR ^= GPIO_Pin_13;
                for (int i=0; i<1000000; i++){
                }
                GPIOC->ODR ^= GPIO_Pin_13;
                for (int i=0; i<1000000; i++){
                }
                GPIOC->ODR ^= GPIO_Pin_13;
                for (int i=0; i<1000000; i++){
                }
                GPIOC->ODR ^= GPIO_Pin_13;
                //uiLedTm = 1000;
                enLedState = 2;
            }
            break;
			
            case 2:
            {
                GPIO_ResetBits(GPIOC, GPIO_Pin_13); // Set C13 to Low level ("0")
		if (ucButtonValue != 0)
		{
		    enLedState = 0;
		}
            }
	    break;
        }
    }
}

/*****************************************************************************************************/

/* Declaration of configuration functions */

void fnvConfigGPIO(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    /* Configure PC.13 pin as PP */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
  
    /* Configure PA.04 pin as in floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);
}

void fnvConfigTimer(void)
{
    TIM_TimeBaseInitTypeDef TIMER_InitStructure;
        
    TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_Prescaler = 65535; //setar de forma a ter 1ms de clock
    TIMER_InitStructure.TIM_Period = 732; //setar de forma a ter 1ms de clock
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIMER_InitStructure);
        
    /* 
    This next line needs to be put here!!!
    That's why the TIM_TimeBaseInit(TIM2, &TIMER_InitStructure); line sets the UIF flag
    We are clearing it!!!
    */
        
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	
    TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Single);        //one time
    //TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Repetitive);  //repetitive 

    // Enable Timer Interrupt , enable timer
    TIM_ITConfig(TIM2, TIM_IT_Update , ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

void fnvConfigIntExt(void)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    /* Configure EXTI4 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

void fnvConfigRCC(void)
{
    /* Enable GPIOC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void fnvConfigNVIC(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    NVIC_InitTypeDef   NVIC_InitStructure;
    /* Enable and set EXTI4 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*****************************************************************************************************/

/* Debounce function */

void fnvInputDeb()
{
    if (uiInDebTm == 0u)
    {
        uiInDebTm = 10u;
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) != 0)
	{
		if (ucButtonPrevValue == 0)
		{
			ucButtonDebCnt = 4;
			ucButtonPrevValue = 1;
		}
	}
        else
	{
		if (ucButtonPrevValue != 0)
		{
			ucButtonDebCnt = 4;
			ucButtonPrevValue = 0;
		}
	}
	    
	if (ucButtonDebCnt != 0)
	{
		ucButtonDebCnt--;
	}
	    
	//if (ucButtonDebCnt == 0)
	else
	{
		ucButtonValue = ucButtonPrevValue;
	}
    }
}

/*****************************************************************************************************/

/* Main function */

void main (void)
{
    /* Configuration functions */
    fnvConfigNVIC();
    fnvConfigRCC();
    fnvConfigTimer();
    fnvConfigIntExt();
    fnvConfigGPIO();
    
    /* Infinite loop */
    while (1)
    {        
        fnvLedCtrl();
        fnvInputDeb();
    }
}

/*****************************************************************************************************/

/* Handlers */

void TIM2_IRQHandler(void)
{
    if (uiLedTm != 0)
    {
        uiLedTm--;
    }
    
    if (uiInDebTm != 0)
    {
        uiInDebTm--;
    }
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
}

void EXTI4_IRQHandler(void)
{
    if (enLedState == 0)
    {
        enLedState = 1;
    }
    EXTI_ClearITPendingBit(EXTI_Line4);
    count++;
}

/*****************************************************************************************************/