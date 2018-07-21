#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"

#include "string.h"

#include "systickdelay.h"
#include "bldc.h"

volatile uint16_t BLDC_Speed = 0;
volatile uint16_t BLDC_Speed_prev = 0;
volatile uint8_t BLDC_COMP_STATE = 0;
volatile uint8_t BEMF_TIMEOUT_COUNTER = 0;
volatile uint8_t BLDC_MotorSpin = 0;
volatile uint8_t BLDC_CURRENT_STEP = 0;
// timing in degrees
volatile uint8_t BLDC_TIMING = BLDC_TIMING_INIT;

uint8_t BLDC_STATE[6] = {0,0,0,0,0,0};

#ifndef BLDC_PWMCOMPLEMENTARYMODE
uint8_t BLDC_STATE_PREV[6] = {0,0,0,0,0,0};
#endif

// BLDC motor steps table
static const uint8_t BLDC_BRIDGE_STATE_SL[6][6] =
{
//	UH,UL		VH,VL	WH,WL
   { 0,1	,	1,0	,	0,0 },
   { 0,1	,	0,0	,	1,0 },
   { 0,0	,	0,1	,	1,0 },
   { 1,0	,	0,1	,	0,0 },
   { 1,0	,	0,0	,	0,1 },
   { 0,0	,	1,0	,	0,1 },
};

static const uint8_t BLDC_PHASE_STATE[6] = {0,1,0,1,0,1};


uint8_t BLDC_MotorSpeedIsOK(void) {
	return ((BLDC_Speed_prev > 0) & (BLDC_Speed > 0));
}

uint16_t BLDC_GetSpeed(void) {
	return BLDC_Speed;
}

// Initialize TIM3, TIM4
void BLDC_SpeedTimerInit(void) {
	TIM_TimeBaseInitTypeDef TIMER_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseStructInit(&TIMER_InitStructure);
	TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIMER_InitStructure.TIM_Prescaler = BLDC_SPEED_TIMER_PRESCALER;
	TIMER_InitStructure.TIM_Period = BLDC_SPEED_TIMER_PERIOD;
	TIM_TimeBaseInit(TIM3, &TIMER_InitStructure);
	TIM_TimeBaseInit(TIM4, &TIMER_InitStructure);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	TIMER_InitStructure.TIM_RepetitionCounter = 1;
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

	TIM_SetCounter(TIM3, 0);
	TIM_SetCounter(TIM4, 0);

	// NVIC Configuration
	// Enable the TIM3_IRQn Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Enable the TIM4_IRQn Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM3_IRQHandler(void) {
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		// Overflow - the motor stopped
		if (BLDC_MotorSpeedIsOK()) {
			BLDC_MotorStop();
		}
	}
}

void TIM4_IRQHandler(void) {
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		TIM_Cmd(TIM4, DISABLE);

		if (BLDC_MotorSpin == BLDC_START) {
			BLDC_MotorCommutation(BLDC_Next_Step());
		}
	}
}

void TIM1_UP_IRQHandler(void) {
	uint8_t STATE;

	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		if (TIM1->CNT < BLDC_CHOPPER_PERIOD / 2 ) {
			if (BEMF_TIMEOUT_COUNTER > BEMF_TIMEOUT) {
				STATE = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14);

				if ((BLDC_COMP_STATE != STATE) & (STATE == BLDC_PHASE_STATE[BLDC_CURRENT_STEP]) ) {
					BLDC_COMP_STATE = STATE;

					BLDC_Speed_prev = BLDC_Speed;
					BLDC_Speed = TIM_GetCounter(TIM3);
		    		TIM_Cmd(TIM3, ENABLE);
					TIM_SetCounter(TIM3, 0);
					// It requires at least two measurement to correct calculate the rotor speed
					if (BLDC_MotorSpeedIsOK()) {
						// Set timer period
						if ((BLDC_TIMING > 0) & (BLDC_TIMING < 31)) {
							TIM4->ARR = 1 + (BLDC_Speed / 2) - BLDC_Speed / (60 / BLDC_TIMING);
						}
						else {
							TIM4->ARR = 1 + (BLDC_Speed / 2);
						}
						TIM_SetCounter(TIM4, 0);
						TIM_Cmd(TIM4, ENABLE);
			    	}
				}
			}
			else {
				BEMF_TIMEOUT_COUNTER++;
			}
		}
	}
}

void BLDC_Init(void) {
	// COMP pin Init
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_14);

	BLDC_PWMTimerInit();
	BLDC_SpeedTimerInit();
	TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_PWM1);
	TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_PWM1);
	TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_PWM1);
	BLDC_MotorStop();
}

void BLDC_PWMTimerInit(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;

	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

	//initialize Tim1 PWM outputs
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	// Time Base configuration
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1; // Center Aligned PWM
	TIM_TimeBaseStructure.TIM_Period = BLDC_CHOPPER_PERIOD;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	// Channel 1, 2, 3  set to PWM mode - all 6 outputs
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0; // initialize to zero output

	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);

	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;


	// DeadTime[ns] = value * (1/SystemCoreFreq) (on 72MHz: 7 is 98ns)
	TIM_BDTRInitStructure.TIM_DeadTime = BLDC_NOL;

	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;

	// Break functionality (overload input)
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;

	TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	// NVIC Configuration
	// Enable the TIM1_IRQn Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM1, ENABLE);
	 // enable motor timer main output (the bridge signals)
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

}

uint8_t BLDC_MotorGetSpin(void) {
	return BLDC_MotorSpin;
}

void BLDC_MotorSetSpin(uint8_t spin) {
	BLDC_MotorSpin = spin;
}

void BLDC_MotorStart(void) {
	uint8_t i;

	BLDC_MotorSpin = BLDC_STOP;
	BLDC_CURRENT_STEP = 0;
	BLDC_MotorCommutation(BLDC_CURRENT_STEP);
	BLDC_SetPWM(BLDC_ADCToPWM(BLDC_ADC_MOTOR_START));
	sysTickDalay(100);

	for (i=0; i < BLDC_START_STEPS; i++) {
		BLDC_MotorCommutation(BLDC_Next_Step());
		if (i==BLDC_START_STEPS-1) {
			BLDC_MotorSpin = BLDC_START;
			break;
		}
		sysTickDalay(BLDC_START_DELAY);
	}
}

void BLDC_MotorStop(void)
{
	BLDC_SetPWM(0);

	TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
	TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
	TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);

	TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
	TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Disable);
	TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Disable);

	BLDC_MotorSpin = BLDC_STOP;
	#ifndef BLDC_PWMCOMPLEMENTARYMODE
		memset(BLDC_STATE_PREV, 0, sizeof(BLDC_STATE_PREV));
	#endif

	TIM_Cmd(TIM3, DISABLE);
	TIM_Cmd(TIM4, DISABLE);
	BLDC_Speed = 0;
	BLDC_Speed_prev = 0;
	BEMF_TIMEOUT_COUNTER = 0;
}

#ifdef BLDC_PWMBOTTOMKEYS
void BLDC_MotorCommutation(uint16_t pos)
{
	memcpy(BLDC_STATE, BLDC_BRIDGE_STATE_SL[pos], sizeof(BLDC_STATE));

	// Disable if need
	if (!BLDC_STATE[UH]) TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
	if (!BLDC_STATE[UL]) TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
	if (!BLDC_STATE[VH]) TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
	if (!BLDC_STATE[VL]) TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Disable);
	if (!BLDC_STATE[WH]) TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
	if (!BLDC_STATE[WL]) TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Disable);

	// Enable if need. If previous state is Enabled then not enable again. Else output do flip-flop.
	if (BLDC_STATE[UH] & !BLDC_STATE[UL] & !BLDC_STATE_PREV[UH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_ForcedAction_Active);
		TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
	}
	if (BLDC_STATE[UL] & !BLDC_STATE[UH] & !BLDC_STATE_PREV[UL]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_PWM1);
		TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
	}
	if (BLDC_STATE[VH] & !BLDC_STATE[VL] & !BLDC_STATE_PREV[VH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_ForcedAction_Active);
		TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);
	}
	if (BLDC_STATE[VL] & !BLDC_STATE[VH] & !BLDC_STATE_PREV[VL]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_PWM1);
		TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);
	}
	if (BLDC_STATE[WH] & !BLDC_STATE[WL] & !BLDC_STATE_PREV[WH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_ForcedAction_Active);
		TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
	}
	if (BLDC_STATE[WL] & !BLDC_STATE[WH] & !BLDC_STATE_PREV[WL]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_PWM1);
		TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);
	}

	memcpy(BLDC_STATE_PREV, BLDC_STATE, sizeof(BLDC_STATE));
	BEMF_TIMEOUT_COUNTER = 0;
}
#endif



#ifdef BLDC_PWMCOMPLEMENTARYMODE
void BLDC_MotorCommutation(uint16_t pos)
{
	memcpy(BLDC_STATE, BLDC_BRIDGE_STATE_SL[pos], sizeof(BLDC_STATE));

	// PWM at low side FET of bridge U
	// active freewheeling at high side FET of bridge U
	// if low side FET is in PWM off mode then the hide side FET
	// is ON for active freewheeling. This mode needs correct definition
	// of dead time otherwise we have shoot-through problems

	if (BLDC_STATE[UH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_PWM1);
		TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
		TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
	} else {
		// Low side FET: OFF
		TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
		if (BLDC_STATE[UL]){
			// High side FET: ON
			TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_ForcedAction_Active);
			TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
		} else {
			// High side FET: OFF
			TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
		}
	}

	if (BLDC_STATE[VH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_PWM1);
		TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);
		TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);
	} else {
		// Low side FET: OFF
		TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
		if (BLDC_STATE[VL]){
			// High side FET: ON
			TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_ForcedAction_Active);
			TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);
		} else {
			// High side FET: OFF
			TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Disable);
		}
	}

	if (BLDC_STATE[WH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_PWM1);
		TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
		TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);
	} else {
		// Low side FET: OFF
		TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
		if (BLDC_STATE[WL]){
			// High side FET: ON
			TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_ForcedAction_Active);
			TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);
		} else {
			// High side FET: OFF
			TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Disable);
		}
	}
	BEMF_TIMEOUT_COUNTER = 0;
}
#endif

uint8_t BLDC_Next_Step() {
	BLDC_CURRENT_STEP++;
	if (BLDC_CURRENT_STEP > 5) BLDC_CURRENT_STEP = 0;
	return BLDC_CURRENT_STEP;
}

uint16_t BLDC_ADCToPWM(uint16_t ADC_VALUE) {
	uint32_t tmp;

	if (ADC_VALUE < BLDC_ADC_STOP) {
		return 0;
	}

	if (ADC_VALUE > BLDC_ADC_MAX) {
		return BLDC_CHOPPER_PERIOD + 1;
	}

	tmp = (uint32_t)(ADC_VALUE-BLDC_ADC_STOP) * (uint32_t)BLDC_CHOPPER_PERIOD / (uint32_t)(BLDC_ADC_MAX - BLDC_ADC_START);

	return (uint16_t) tmp;
}


void BLDC_SetPWM(uint16_t PWM)
{
	TIM1->CCR1 = PWM;
	TIM1->CCR2 = PWM;
	TIM1->CCR3 = PWM;
}
