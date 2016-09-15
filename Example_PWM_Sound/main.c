#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

#define SYSCLK 72000000
#define PRESCALER 72

#define C	261	//Do
#define C_	277 //Do#
#define D	293 //Re
#define D_	311 //Re#
#define E	239 //Mi
#define F	349 //Fa
#define F_	370 //Fa#
#define G 	392 //Sol
#define G_	415 //Sol#
#define A	440 //La
#define A_	466 //La#
#define H	494 //Si

#define t1		2000
#define t2		1000
#define t4		500
#define t8		250
#define t16		125

typedef struct
{
	uint16_t freq;
	uint16_t time;
}SoundTypeDef;

#define MUSICSIZE 48

const SoundTypeDef Music[MUSICSIZE] ={
	{C*2, t4},
	{G, t4},
	{A_, t8},
	{F, t8},
	{D_, t8},
	{F, t8},
	{G, t4},
	{C, t2},
	{C*2, t4},
	{G, t4},
	{A_, t8},
	{F, t8},
	{D_, t8},
	{F, t8},
	{G, t4},
	{C*2, t4},
	{0, t8},
	{D_, t8},
	{D_, t8},
	{D_, t8},
	{G, t8},
	{A_, t4},
	{D_*2, t8},
	{C_*2, t8},
	{C*2, t8},
	{C*2, t8},
	{C*2, t8},
	{C*2, t8},
	{A_, t8},
	{F, t8},
	{D_, t8},
	{F, t8},
	{G, t4},
	{C*2, t2},
	{C*2, t2},
	{A_, t8},
	{G_, t8},
	{G, t8},
	{G_, t8},
	{A_, t2},
	{A_, t4},
	{C*2, t4},
	{A_, t8},
	{F, t8},
	{D_, t8},
	{F, t8},
	{G, t4},
	{C*2, t2}
};

int MusicStep = 0;
char PlayMusic = 0;

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

GPIO_InitTypeDef port;
TIM_TimeBaseInitTypeDef timer;
TIM_OCInitTypeDef timerPWM;

int sound_time;
int sound_counter;

void sound_init(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	GPIO_StructInit(&port);
	port.GPIO_Mode = GPIO_Mode_AF_PP;
	port.GPIO_Pin = GPIO_Pin_6;
	port.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &port);

	TIM_TimeBaseStructInit(&timer);
	timer.TIM_Prescaler = PRESCALER;
	timer.TIM_Period = 0xFFFF;
	timer.TIM_ClockDivision = 0;
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &timer);

	TIM_OCStructInit(&timerPWM);
	timerPWM.TIM_Pulse = 0;
	timerPWM.TIM_OCMode = TIM_OCMode_PWM1;
	timerPWM.TIM_OutputState = TIM_OutputState_Enable;
	timerPWM.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM4, &timerPWM);

    /* Enable Interrupt by overflow */
    TIM_ITConfig(TIM4, TIM_IT_CC4, ENABLE);

	//TIM_Cmd(TIM4, ENABLE);

    /* Enable Interrupt of Timer TIM2 */
    NVIC_EnableIRQ(TIM4_IRQn);
}

void sound (int freq, int time_ms) {
	if (freq > 0) {
		TIM4->ARR = SYSCLK / timer.TIM_Prescaler / freq;
		TIM4->CCR1 = TIM4->ARR / 2;
	}
	else {
		TIM4->ARR = 1000;
		TIM4->CCR1 = 0;
	}
	TIM_SetCounter(TIM4, 0);

	sound_time = ((SYSCLK / timer.TIM_Prescaler / TIM4->ARR) * time_ms ) / 1000;
	sound_counter = 0;
	TIM_Cmd(TIM4, ENABLE);
}

void StartMusic(void) {
	MusicStep = 0;
	PlayMusic = 1;
	sound(Music[MusicStep].freq, Music[MusicStep].time);
}

void TIM4_IRQHandler(void){

	if (TIM_GetITStatus(TIM4, TIM_IT_CC4) != RESET)
	  {
	    /* Reset flag */
	    TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);

	    sound_counter++;
	    if (sound_counter > sound_time) {
	    	if (PlayMusic == 0) {
	    		TIM_Cmd(TIM4, DISABLE);
	    	}
	    	else {
	    		if (MusicStep < MUSICSIZE-1) {
	    			if (TIM4->CCR1 == 0){
	    				MusicStep++;
	    				sound(Music[MusicStep].freq, Music[MusicStep].time);
	    			}
	    			else{
	    				sound(0, 30);
	    			}
	    		}
	    		else {
		    		PlayMusic = 0;
		    		TIM_Cmd(TIM4, DISABLE);
	    		}
	    	}
	    }

	    /* over-capture */
	    if (TIM_GetFlagStatus(TIM4, TIM_FLAG_CC4OF) != RESET)
	    {
	      TIM_ClearFlag(TIM4, TIM_FLAG_CC4OF);
	      // ...
	    }
	  }
}


int main(void)
{
	SetSysClockTo72();
	sound_init();

	//sound (440, 1000);
	StartMusic();

	while(1)
    {

    }

}
