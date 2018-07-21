#ifndef _BLDC_LIB_H_
#define _BLDC_LIB_H_

//============= Settings ===============
// PWM Frequency = 72000000/2/BLDC_CHOPPER_PERIOD
#define BLDC_CHOPPER_PERIOD 2250

// Dead time = BLDC_NOL/72000000  (on 72MHz: 7 is 98ns)
// (on 72MHz: 72 is 1000ns)
#define BLDC_NOL 72

// You must define BLDC_PWMBOTTOMKEYS  OR  BLDC_PWMBOTTOMKEYS
//#define BLDC_PWMBOTTOMKEYS
#define BLDC_PWMCOMPLEMENTARYMODE

// ADC limits
#define BLDC_ADC_MOTOR_START 700 // limit for open loop start
#define BLDC_ADC_START 700
#define BLDC_ADC_STOP 200
#define BLDC_ADC_MAX 4000
#define BLDC_ADC_ZERO 50

#define BEMF_TIMEOUT 1 // BEMF filter 0..5

#define BLDC_START_STEPS 24
#define BLDC_START_DELAY 10

#define BLDC_TIMING_INIT 10 // timing in degrees 0..30

#define BLDC_SPEED_TIMER_PRESCALER	36
#define BLDC_SPEED_TIMER_PERIOD	0xFFFF

//==============================================

#define BLDC_STOP	0
#define BLDC_START	1

#define UH	0
#define UL	1
#define VH	2
#define VL	3
#define WH	4
#define WL	5

void BLDC_SpeedTimerInit(void);

void BLDC_Init(void);
void BLDC_PWMTimerInit(void);
void BLDC_MotorSetSpin(uint8_t spin);
uint8_t BLDC_MotorGetSpin(void);
void BLDC_MotorStart(void);
void BLDC_MotorStop(void);
void BLDC_MotorCommutation(uint16_t pos);
uint8_t BLDC_Next_Step();
uint16_t BLDC_ADCToPWM(uint16_t ADC_VALUE);
void BLDC_SetPWM(uint16_t PWM);
void BLDC_ApplyPWM();

#endif
