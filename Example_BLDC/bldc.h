#ifndef _BLDC_LIB_H_
#define _BLDC_LIB_H_

// PWM Frequency = 72000000/BLDC_CHOPPER_PERIOD
#define BLDC_CHOPPER_PERIOD 4500
// Dead time = BLDC_NOL/72000000  (on 72MHz: 7 is 98ns)
// (on 72MHz: 72 is 1000ns)
//#define BLDC_NOL 72
#define BLDC_NOL 72

//#define BLDC_PWMTOPKEYS
//#define BLDC_PWMBOTTOMKEYS
//#define BLDC_PWMTOPBOTTOMKEYS
#define BLDC_PWMCOMPLEMENTARYMODE

#define BLDC_ADC_START 200
#define BLDC_ADC_STOP 50
#define BLDC_ADC_MAX 4000

/////////////////////////////////////////////////////////////////////////
#define BLDC_STOP	0
#define BLDC_CW		1
#define BLDC_CCW	2

#define UH	0
#define UL	1
#define VH	2
#define VL	3
#define WH	4
#define WL	5

void BLDC_Init(void);
void BLDC_HallSensorsInit(void);
void BLDC_PWMTimerInit(void);
uint8_t BLDC_HallSensorsGetPosition(void);
void BLDC_MotorSetSpin(uint8_t spin);
uint8_t BLDC_MotorGetSpin(void);
void BLDC_MotorStop(void);
void BLDC_MotorCommutation(uint16_t hallpos);
uint16_t BLDC_ADCToPWM(uint16_t ADC_VALUE);
void BLDC_SetPWM(uint16_t PWM);

#endif
