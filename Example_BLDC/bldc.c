#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "string.h"
#include "stdio.h"

#include "bldc.h"

uint8_t BLDC_MotorSpin = 0;
uint8_t BLDC_STATE[6] = {0,0,0,0,0,0};

#ifndef BLDC_PWMCOMPLEMENTARYMODE
uint8_t BLDC_STATE_PREV[6] = {0,0,0,0,0,0};
#endif

// BLDC motor steps tables
static const uint8_t BLDC_BRIDGE_STATE_FORWARD[8][6] =   // Motor steps
{
//	UH,UL		VH,VL	WH,WL
   { 0,0	,	0,0	,	0,0 },  // 0 //000
   { 0,1	,	0,0	,	1,0 },
   { 1,0	,	0,1	,	0,0 },
   { 0,0	,	0,1	,	1,0 },
   { 0,0	,	1,0	,	0,1 },
   { 0,1	,	1,0	,	0,0 },
   { 1,0	,	0,0	,	0,1 },
   { 0,0	,	0,0	,	0,0 },  // 0 //111
};

static const uint8_t BLDC_BRIDGE_STATE_BACKWARD[8][6] =   // Motor steps
{
//	UH,UL		VH,VL	WH,WL
   { 0,0	,	0,0	,	0,0 },  //  //000
   { 1,0	,	0,0	,	0,1 },
   { 0,1	,	1,0	,	0,0 },
   { 0,0	,	1,0	,	0,1 },
   { 0,0	,	0,1	,	1,0 },
   { 1,0	,	0,1	,	0,0 },
   { 0,1	,	0,0	,	1,0 },
   { 0,0	,	0,0	,	0,0 },  //  //111
};

void BLDC_Init(void) {
	BLDC_HallSensorsInit();
	BLDC_PWMTimerInit();
	TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_PWM1);
	TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_PWM1);
	TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_PWM1);
	BLDC_MotorStop();
}

void BLDC_HallSensorsInit(void) {
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	// Enable clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	// Init GPIO
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Init NVIC
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

    // Tell system that you will use EXTI_Lines */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);

    // EXTI
    EXTI_InitStruct.EXTI_Line = EXTI_Line7 | EXTI_Line8 | EXTI_Line9;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&EXTI_InitStruct);
}

void EXTI9_5_IRQHandler(void) {
    if ((EXTI_GetITStatus(EXTI_Line7) | EXTI_GetITStatus(EXTI_Line8) | EXTI_GetITStatus(EXTI_Line9)) != RESET) {
    	// Clear interrupt flags
    	EXTI_ClearITPendingBit(EXTI_Line7);
    	EXTI_ClearITPendingBit(EXTI_Line8);
    	EXTI_ClearITPendingBit(EXTI_Line9);

        // Commutation
        BLDC_MotorCommutation(BLDC_HallSensorsGetPosition());
    }
}

void BLDC_PWMTimerInit(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;

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
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = BLDC_CHOPPER_PERIOD;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	// Channel 1, 2, 3 – set to PWM mode - all 6 outputs
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0; // initialize to zero output

	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset; /// !!!
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset; /// !!!

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

	TIM_Cmd(TIM1, ENABLE);
	 // enable motor timer main output (the bridge signals)
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

uint8_t BLDC_HallSensorsGetPosition(void) {
	return (uint8_t) ((GPIO_ReadInputData(GPIOB) & (GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9)) >> 7);
}

uint8_t BLDC_MotorGetSpin(void) {
	return BLDC_MotorSpin;
}

void BLDC_MotorSetSpin(uint8_t spin) {
	BLDC_MotorSpin = spin;
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

	BLDC_MotorSpin = 0;
	#ifndef BLDC_PWMCOMPLEMENTARYMODE
		memset(BLDC_STATE_PREV, 0, sizeof(BLDC_STATE_PREV));
	#endif
}

#ifdef BLDC_PWMTOPBOTTOMKEYS
void BLDC_MotorCommutation(uint16_t hallpos)
{
	if (BLDC_MotorSpin == BLDC_CW) {
		memcpy(BLDC_STATE, BLDC_BRIDGE_STATE_FORWARD[hallpos], sizeof(BLDC_STATE));
	}
	else {
		memcpy(BLDC_STATE, BLDC_BRIDGE_STATE_BACKWARD[hallpos], sizeof(BLDC_STATE));
	}

	// Disable if need
	if (!BLDC_STATE[UH]) TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
	if (!BLDC_STATE[UL]) TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
	if (!BLDC_STATE[VH]) TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
	if (!BLDC_STATE[VL]) TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Disable);
	if (!BLDC_STATE[WH]) TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
	if (!BLDC_STATE[WL]) TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Disable);

	// Enable if need. If previous state is Enabled then not enable again. Else output do flip-flop.
	if (BLDC_STATE[UH] & !BLDC_STATE[UL] & !BLDC_STATE_PREV[UH]) TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
	if (BLDC_STATE[UL] & !BLDC_STATE[UH] & !BLDC_STATE_PREV[UL]) TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
	if (BLDC_STATE[VH] & !BLDC_STATE[VL] & !BLDC_STATE_PREV[VH]) TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);
	if (BLDC_STATE[VL] & !BLDC_STATE[VH] & !BLDC_STATE_PREV[VL]) TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);
	if (BLDC_STATE[WH] & !BLDC_STATE[WL] & !BLDC_STATE_PREV[WH]) TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
	if (BLDC_STATE[WL] & !BLDC_STATE[WH] & !BLDC_STATE_PREV[WL]) TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);

	memcpy(BLDC_STATE_PREV, BLDC_STATE, sizeof(BLDC_STATE));
}
#endif

#ifdef BLDC_PWMTOPKEYS
void BLDC_MotorCommutation(uint16_t hallpos)
{
	if (BLDC_MotorSpin == BLDC_CW) {
		memcpy(BLDC_STATE, BLDC_BRIDGE_STATE_FORWARD[hallpos], sizeof(BLDC_STATE));
	}
	else {
		memcpy(BLDC_STATE, BLDC_BRIDGE_STATE_BACKWARD[hallpos], sizeof(BLDC_STATE));
	}

	// Disable if need
	if (!BLDC_STATE[UH]) TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
	if (!BLDC_STATE[UL]) TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
	if (!BLDC_STATE[VH]) TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
	if (!BLDC_STATE[VL]) TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Disable);
	if (!BLDC_STATE[WH]) TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
	if (!BLDC_STATE[WL]) TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Disable);

	// Enable if need. If previous state is Enabled then not enable again. Else output do flip-flop.
	if (BLDC_STATE[UH] & !BLDC_STATE[UL] & !BLDC_STATE_PREV[UH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_PWM1);
		TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
	}
	if (BLDC_STATE[UL] & !BLDC_STATE[UH] & !BLDC_STATE_PREV[UL]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_ForcedAction_Active);
		TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
	}
	if (BLDC_STATE[VH] & !BLDC_STATE[VL] & !BLDC_STATE_PREV[VH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_PWM1);
		TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);
	}
	if (BLDC_STATE[VL] & !BLDC_STATE[VH] & !BLDC_STATE_PREV[VL]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_ForcedAction_Active);
		TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);
	}
	if (BLDC_STATE[WH] & !BLDC_STATE[WL] & !BLDC_STATE_PREV[WH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_PWM1);
		TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
	}
	if (BLDC_STATE[WL] & !BLDC_STATE[WH] & !BLDC_STATE_PREV[WL]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_ForcedAction_Active);
		TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);
	}

	memcpy(BLDC_STATE_PREV, BLDC_STATE, sizeof(BLDC_STATE));
}
#endif

#ifdef BLDC_PWMBOTTOMKEYS
void BLDC_MotorCommutation(uint16_t hallpos)
{
	if (BLDC_MotorSpin == BLDC_CW) {
		memcpy(BLDC_STATE, BLDC_BRIDGE_STATE_FORWARD[hallpos], sizeof(BLDC_STATE));
	}
	else {
		memcpy(BLDC_STATE, BLDC_BRIDGE_STATE_BACKWARD[hallpos], sizeof(BLDC_STATE));
	}

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
}
#endif



#ifdef BLDC_PWMCOMPLEMENTARYMODE
void BLDC_MotorCommutation(uint16_t hallpos) {
	if (BLDC_MotorSpin == BLDC_CW) {
		memcpy(BLDC_STATE, BLDC_BRIDGE_STATE_FORWARD[hallpos], sizeof(BLDC_STATE));
	}
	else {
		memcpy(BLDC_STATE, BLDC_BRIDGE_STATE_BACKWARD[hallpos], sizeof(BLDC_STATE));
	}

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

}
#endif

uint16_t BLDC_ADCToPWM(uint16_t ADC_VALUE) {
	uint32_t tmp;

	if (ADC_VALUE < BLDC_ADC_STOP) {
		return 0;
	}

	if (ADC_VALUE > BLDC_ADC_MAX) {
		return BLDC_CHOPPER_PERIOD+1;
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

