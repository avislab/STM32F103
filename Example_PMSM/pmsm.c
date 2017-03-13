#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "string.h"
#include "stdio.h"

#include "pmsm.h"

void TIM4_IRQHandler(void);

// Variables
volatile uint8_t PMSM_MotorRunFlag = 0;
volatile uint8_t PMSM_MotorSpin = PMSM_CW;
uint8_t PMSM_State[6] = {0, 0, 0, 0, 0, 0};
volatile uint8_t	PMSM_Sensors = 0;
volatile uint8_t PMSM_SinTableIndex = 0;
volatile uint16_t PMSM_PWM = 0;
volatile uint16_t PMSM_Speed = 0;
volatile uint16_t PMSM_Speed_prev = 0;
volatile uint8_t PMSM_ModeEnabled = 0;
// Timing (points in sine table)
// sine table contains 192 items; 360/192 = 1.875 degrees per item
volatile static int8_t PMSM_Timing = 10; // 15 * 1.875 = 28.125 degrees

// Forward Motor steps
static const uint8_t PMSM_BRIDGE_STATE_FORWARD[8][6] =
{
//	UH,UL		VH,VL	WH,WL
   { 0,0,		0,0,	0,0 },  // 0 //000
   { 0,1,		0,0,	1,0 },
   { 1,0,		0,1,	0,0 },
   { 0,0,		0,1,	1,0 },
   { 0,0,		1,0,	0,1 },
   { 0,1,		1,0,	0,0 },
   { 1,0,		0,0,	0,1 },
   { 0,0,		0,0,	0,0 },  // 0 //111
};

// Backward Motor steps
static const uint8_t PMSM_BRIDGE_STATE_BACKWARD[8][6] =
{
//	UH,UL		VH,VL	WH,WL
   { 0,0,		0,0,	0,0 },  //  //000
   { 1,0,		0,0,	0,1 },
   { 0,1,		1,0,	0,0 },
   { 0,0,		1,0,	0,1 },
   { 0,0,		0,1,	1,0 },
   { 1,0,		0,1,	0,0 },
   { 0,1,		0,0,	1,0 },
   { 0,0,		0,0,	0,0 },  //  //111
};

// Sin table
#define PMSM_SINTABLESIZE	192
static const uint8_t PMSM_SINTABLE [PMSM_SINTABLESIZE][3] =
{
		{0,       0,      221},
		{8,       0,      225},
		{17,      0,      229},
		{25,      0,      232},
		{33,      0,      236},
		{42,      0,      239},
		{50,      0,      241},
		{58,      0,      244},
		{66,      0,      246},
		{74,      0,      248},
		{82,      0,      250},
		{90,      0,      252},
		{98,      0,      253},
		{105,     0,      254},
		{113,     0,      254},
		{120,     0,      255},
		{128,     0,      255},
		{135,     0,      255},
		{142,     0,      254},
		{149,     0,      254},
		{155,     0,      253},
		{162,     0,      252},
		{168,     0,      250},
		{174,     0,      248},
		{180,     0,      246},
		{186,     0,      244},
		{192,     0,      241},
		{197,     0,      239},
		{202,     0,      236},
		{207,     0,      232},
		{212,     0,      229},
		{217,     0,      225},
		{221,     0,      221},
		{225,     0,      217},
		{229,     0,      212},
		{232,     0,      207},
		{236,     0,      202},
		{239,     0,      197},
		{241,     0,      192},
		{244,     0,      186},
		{246,     0,      180},
		{248,     0,      174},
		{250,     0,      168},
		{252,     0,      162},
		{253,     0,      155},
		{254,     0,      149},
		{254,     0,      142},
		{255,     0,      135},
		{255,     0,      127},
		{255,     0,      120},
		{254,     0,      113},
		{254,     0,      105},
		{253,     0,      98},
		{252,     0,      90},
		{250,     0,      82},
		{248,     0,      74},
		{246,     0,      66},
		{244,     0,      58},
		{241,     0,      50},
		{239,     0,      42},
		{236,     0,      33},
		{232,     0,      25},
		{229,     0,      17},
		{225,     0,      8},
		{221,     0,      0},
		{225,     8,      0},
		{229,     17,     0},
		{232,     25,     0},
		{236,     33,     0},
		{239,     42,     0},
		{241,     50,     0},
		{244,     58,     0},
		{246,     66,     0},
		{248,     74,     0},
		{250,     82,     0},
		{252,     90,     0},
		{253,     98,     0},
		{254,     105,    0},
		{254,     113,    0},
		{255,     120,    0},
		{255,     127,    0},
		{255,     135,    0},
		{254,     142,    0},
		{254,     149,    0},
		{253,     155,    0},
		{252,     162,    0},
		{250,     168,    0},
		{248,     174,    0},
		{246,     180,    0},
		{244,     186,    0},
		{241,     192,    0},
		{239,     197,    0},
		{236,     202,    0},
		{232,     207,    0},
		{229,     212,    0},
		{225,     217,    0},
		{221,     221,    0},
		{217,     225,    0},
		{212,     229,    0},
		{207,     232,    0},
		{202,     236,    0},
		{197,     239,    0},
		{192,     241,    0},
		{186,     244,    0},
		{180,     246,    0},
		{174,     248,    0},
		{168,     250,    0},
		{162,     252,    0},
		{155,     253,    0},
		{149,     254,    0},
		{142,     254,    0},
		{135,     255,    0},
		{128,     255,    0},
		{120,     255,    0},
		{113,     254,    0},
		{105,     254,    0},
		{98,      253,    0},
		{90,      252,    0},
		{82,      250,    0},
		{74,      248,    0},
		{66,      246,    0},
		{58,      244,    0},
		{50,      241,    0},
		{42,      239,    0},
		{33,      236,    0},
		{25,      232,    0},
		{17,      229,    0},
		{8,       225,    0},
		{0,       221,    0},
		{0,       225,    8},
		{0,       229,    17},
		{0,       232,    25},
		{0,       236,    33},
		{0,       239,    42},
		{0,       241,    50},
		{0,       244,    58},
		{0,       246,    66},
		{0,       248,    74},
		{0,       250,    82},
		{0,       252,    90},
		{0,       253,    98},
		{0,       254,    105},
		{0,       254,    113},
		{0,       255,    120},
		{0,       255,    128},
		{0,       255,    135},
		{0,       254,    142},
		{0,       254,    149},
		{0,       253,    155},
		{0,       252,    162},
		{0,       250,    168},
		{0,       248,    174},
		{0,       246,    180},
		{0,       244,    186},
		{0,       241,    192},
		{0,       239,    197},
		{0,       236,    202},
		{0,       232,    207},
		{0,       229,    212},
		{0,       225,    217},
		{0,       221,    221},
		{0,       217,    225},
		{0,       212,    229},
		{0,       207,    232},
		{0,       202,    236},
		{0,       197,    239},
		{0,       192,    241},
		{0,       186,    244},
		{0,       180,    246},
		{0,       174,    248},
		{0,       168,    250},
		{0,       162,    252},
		{0,       155,    253},
		{0,       149,    254},
		{0,       142,    254},
		{0,       135,    255},
		{0,       128,    255},
		{0,       120,    255},
		{0,       113,    254},
		{0,       105,    254},
		{0,       98,     253},
		{0,       90,     252},
		{0,       82,     250},
		{0,       74,     248},
		{0,       66,     246},
		{0,       58,     244},
		{0,       50,     241},
		{0,       42,     239},
		{0,       33,     236},
		{0,       25,     232},
		{0,       17,     229},
		{0,       8,      225}
};

// Phase correction table
static const uint8_t PMSM_STATE_TABLE_INDEX_FORWARD[8] = {0, 160, 32, 0, 96, 128, 64, 0};
static const uint8_t PMSM_STATE_TABLE_INDEX_BACKWARD[8] = {0, 32, 160, 0, 96, 64, 128, 0};

#define UH	0
#define UL	1
#define VH	2
#define VL	3
#define WH	4
#define WL	5

// Initialize of all needed peripheral
void PMSM_Init(void) {
	PMSM_HallSensorsInit();
	PMSM_PWMTimerInit();
	TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_PWM1);
	TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_PWM1);
	TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_PWM1);
	PMSM_SinTimerInit();
	PMSM_SpeedTimerInit();
	PMSM_MotorStop();
}

// Stop a motor
void PMSM_MotorStop(void)
{
	PMSM_SetPWM(0);

	TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
	TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
	TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);

	TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
	TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Disable);
	TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Disable);

	TIM_Cmd(TIM3, DISABLE);
	TIM_Cmd(TIM4, DISABLE);
	PMSM_Speed = 0;
	PMSM_Speed_prev = 0;
	PMSM_MotorRunFlag = 0;
	PMSM_ModeEnabled = 0;
}

// Configure GPIO, NVIC, EXTI for 3 Hall sensors
void PMSM_HallSensorsInit(void) {
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	// Enable clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	// Init GPIO
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Init NVIC
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

    // Tell system that you will use PB lines for EXTI
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

// Every time when hall sensors change state executed this IRQ handler
void EXTI9_5_IRQHandler(void) {
    if ((EXTI_GetITStatus(EXTI_Line7) | EXTI_GetITStatus(EXTI_Line8) | EXTI_GetITStatus(EXTI_Line9)) != RESET) {
    	// Clear interrupt flag
    	EXTI_ClearITPendingBit(EXTI_Line7);
    	EXTI_ClearITPendingBit(EXTI_Line8);
    	EXTI_ClearITPendingBit(EXTI_Line9);

    	// Get current rotor position
    	PMSM_Sensors = PMSM_HallSensorsGetPosition();

//    	if (PMSM_Sensors == 1) {
    		// Get rotation time (in inverse ratio speed) from timer TIM3
    		PMSM_Speed_prev = PMSM_Speed;
    		PMSM_Speed = TIM_GetCounter(TIM3);
    		TIM_Cmd(TIM3, ENABLE);
			TIM_SetCounter(TIM3, 0);

			// It requires at least two measurement to correct calculate the rotor speed
			if (PMSM_MotorSpeedIsOK()) {
				// Enable timer TIM4 to generate sine
				TIM_SetCounter(TIM4, 0);
				// Set timer period
//				TIM4->ARR = PMSM_Speed / PMSM_SINTABLESIZE;
				TIM4->ARR = PMSM_Speed / 32; //32 - number of items in the sine table between commutations (192/6 = 32)
				TIM_Cmd(TIM4, ENABLE);
//			}
    	}

		// If Hall sensors value is valid
    	if ((PMSM_Sensors > 0 ) & (PMSM_Sensors < 7)) {
    		// Do a phase correction
    		PMSM_SinTableIndex = PMSM_GetState(PMSM_Sensors);
		}

    	// If motor is started then used a block commutation
    	if (PMSM_ModeEnabled == 0) {
    		PMSM_MotorCommutation(PMSM_Sensors);
    	}
    }
}

// Initialize Timer TIM1 & PWM output. Timer TIM1 generate 6-PWM outputs
void PMSM_PWMTimerInit(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

	// Initialize Tim1 PWM outputs
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
	TIM_TimeBaseStructure.TIM_Period = PMSM_CHOPPER_PERIOD;
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
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset; ///
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset; ///

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);

	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;

	// DeadTime[ns] = value * (1/SystemCoreFreq) (on 72MHz: 7 is 98ns)
	TIM_BDTRInitStructure.TIM_DeadTime = PMSM_NOL;

	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;

	// Break functionality
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;

	TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);

	TIM_Cmd(TIM1, ENABLE);
	 // Enable motor timer main output (the bridge signals)
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

// Initialize TIM4 which generate 3-phase sine signal
void PMSM_SinTimerInit(void) {
	TIM_TimeBaseInitTypeDef TIMER_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseStructInit(&TIMER_InitStructure);
	TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIMER_InitStructure.TIM_Prescaler = PMSM_SPEED_TIMER_PRESCALER;
	TIMER_InitStructure.TIM_Period = 0;
	TIM_TimeBaseInit(TIM4, &TIMER_InitStructure);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	//TIM_Cmd(TIM4, ENABLE);

	// NVIC Configuration
	// Enable the TIM4_IRQn Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// Calculate 3-phase PWM and increment position in sine table
void TIM4_IRQHandler(void) {
	uint16_t PWM1, PWM2, PWM3;

	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

		// If time to enable PMSM mode
		if (PMSM_ModeEnabled == 0) {
			// Turn PWM outputs for working with sine wave
			TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_PWM1);
			TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
			TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);

			TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_PWM1);
			TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);
			TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);

			TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_PWM1);
			TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
			TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);

			PMSM_ModeEnabled = 1;
		}

		// Calculate PWM for 3-phase
		PWM1 = (uint16_t)((uint32_t)PMSM_PWM * PMSM_SINTABLE[PMSM_SinTableIndex][0]/255);
		PWM2 = (uint16_t)((uint32_t)PMSM_PWM * PMSM_SINTABLE[PMSM_SinTableIndex][1]/255);
		PWM3 = (uint16_t)((uint32_t)PMSM_PWM * PMSM_SINTABLE[PMSM_SinTableIndex][2]/255);

		if (PMSM_MotorSpin == PMSM_CW) {
			// Forward rotation
			PMSM_SetPWM_UVW(PWM1, PWM2, PWM3);
		}
		else {
			// Backward rotation
			PMSM_SetPWM_UVW(PWM1, PWM3, PWM2);
		}

		// Increment position in sine table
		PMSM_SinTableIndex++;
		if (PMSM_SinTableIndex > PMSM_SINTABLESIZE-1) {
			PMSM_SinTableIndex = 0;
		}
	}
}

// Initialize TIM3. It used to calculate the speed
void PMSM_SpeedTimerInit(void) {
	TIM_TimeBaseInitTypeDef TIMER_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructInit(&TIMER_InitStructure);
	TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIMER_InitStructure.TIM_Prescaler = PMSM_SPEED_TIMER_PRESCALER;
	TIMER_InitStructure.TIM_Period = PMSM_SPEED_TIMER_PERIOD;
	TIM_TimeBaseInit(TIM3, &TIMER_InitStructure);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM3, 0);
	//TIM_Cmd(TIM3, ENABLE);

	// NVIC Configuration
	// Enable the TIM3_IRQn Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM3_IRQHandler(void) {
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		// Overflow - the motor is stopped
		if (PMSM_MotorSpeedIsOK()) {
			PMSM_MotorStop();
		}
	}
}

// Get data from hall sensors
uint8_t PMSM_HallSensorsGetPosition(void) {
	return (uint8_t) ((GPIO_ReadInputData(GPIOB) & (GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9)) >> 7);
}


// This commutation is used just when motor start
void PMSM_MotorCommutation(uint16_t hallpos) {
	if (PMSM_MotorSpin == PMSM_CW) {
		memcpy(PMSM_State, PMSM_BRIDGE_STATE_FORWARD[hallpos], sizeof(PMSM_State));
	}
	else {
		memcpy(PMSM_State, PMSM_BRIDGE_STATE_BACKWARD[hallpos], sizeof(PMSM_State));
	}

	// PWM at low side FET of bridge U
	// active freewheeling at high side FET of bridge U
	// if low side FET is in PWM off mode then the hide side FET
	// is ON for active freewheeling. This mode needs correct definition
	// of dead time otherwise we have shoot-through problems

	if (PMSM_State[UH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_PWM1);
		TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
		TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
	} else {
		// Low side FET: OFF
		TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
		if (PMSM_State[UL]){
			// High side FET: ON
			TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_ForcedAction_Active);
			TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
		} else {
			// High side FET: OFF
			TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
		}
	}

	if (PMSM_State[VH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_PWM1);
		TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);
		TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);
	} else {
		// Low side FET: OFF
		TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
		if (PMSM_State[VL]){
			// High side FET: ON
			TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_ForcedAction_Active);
			TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);
		} else {
			// High side FET: OFF
			TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Disable);
		}
	}

	if (PMSM_State[WH]) {
		TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_PWM1);
		TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
		TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);
	} else {
		// Low side FET: OFF
		TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
		if (PMSM_State[WL]){
			// High side FET: ON
			TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_ForcedAction_Active);
			TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);
		} else {
			// High side FET: OFF
			TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Disable);
		}
	}

}

// Transform ADC value to value for writing to the timer register
uint16_t PMSM_ADCToPWM(uint16_t ADC_VALUE) {
	uint32_t tmp;

	if (ADC_VALUE < PMSM_ADC_STOP) {
		return 0;
	} else {
		if (ADC_VALUE > PMSM_ADC_MAX) {
			return PMSM_CHOPPER_PERIOD+1;
		}
		else {
			tmp = (uint32_t)(ADC_VALUE-PMSM_ADC_STOP) * (uint32_t)PMSM_CHOPPER_PERIOD / (uint32_t)(PMSM_ADC_MAX - PMSM_ADC_START);
			return (uint16_t) tmp;
		}
	}
}

// Set PWM (same for all phases)
void PMSM_SetPWM(uint16_t PWM)
{
	if (PMSM_ModeEnabled == 0) {
		TIM1->CCR1 = PWM;
		TIM1->CCR2 = PWM;
		TIM1->CCR3 = PWM;
	}
	else {
		PMSM_PWM = PWM;
	}
}

// Set PWM
void PMSM_SetPWM_UVW(uint16_t PWM1, uint16_t PWM2, uint16_t PWM3)
{
	if (PMSM_ModeEnabled == 1) {
		TIM1->CCR1 = PWM1;
		TIM1->CCR2 = PWM2;
		TIM1->CCR3 = PWM3;
	}
}

// Get index in sine table based on the sensor data, the timing and the direction of rotor rotation
uint8_t	PMSM_GetState(uint8_t SensorsPosition) {
	int16_t index;

	if (PMSM_MotorSpin == PMSM_CW) {
		index = PMSM_STATE_TABLE_INDEX_FORWARD[SensorsPosition];
	}
	else {
		index = PMSM_STATE_TABLE_INDEX_BACKWARD[SensorsPosition];
	}

	index = index + (int16_t)PMSM_Timing;
	if (index > PMSM_SINTABLESIZE-1) {
		index = index - PMSM_SINTABLESIZE;
	}
	else {
		if (index < 0) {
			index = PMSM_SINTABLESIZE + index;
		}
	}

	return index;
}

uint8_t PMSM_MotorIsRun(void) {
	return PMSM_MotorRunFlag;
}

uint8_t PMSM_MotorSpeedIsOK(void) {
	return ((PMSM_Speed_prev > 0) & (PMSM_Speed > 0));
}

uint16_t PMSM_GetSpeed(void) {
	return PMSM_Speed;
}

void PMSM_MotorSetRun(void) {
	PMSM_MotorRunFlag = 1;
}

void PMSM_MotorSetSpin(uint8_t spin) {
	PMSM_MotorSpin = spin;
}
