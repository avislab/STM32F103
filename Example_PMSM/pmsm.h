#ifndef _PMSM_LIB_H_
#define _PMSM_LIB_H_

// PWM Frequency = 72000000/PMSM_CHOPPER_PERIOD
#define PMSM_CHOPPER_PERIOD 4500
//#define PMSM_CHOPPER_PERIOD 2250
//#define PMSM_CHOPPER_PERIOD 1125
// Dead time = PMSM_NOL/72000000  (on 72MHz: 7 is 98ns)
// (on 72MHz: 72 is 1000ns)
//#define PMSM_NOL 72
#define PMSM_NOL 72

#define PMSM_ADC_START 200
#define PMSM_ADC_STOP 50
#define PMSM_ADC_MAX 4000

#define PMSM_SPEED_TIMER_PRESCALER	72
#define PMSM_SPEED_TIMER_PERIOD	0xFFFF // 65535

#define PMSM_CW		0
#define PMSM_CCW	1

void PMSM_Init(void);
void PMSM_MotorSetSpin(uint8_t spin);
void PMSM_MotorSetRun(void);
void PMSM_MotorStop(void);
uint8_t PMSM_MotorIsRun(void);
void PMSM_MotorCommutation(uint16_t hallpos);
uint8_t PMSM_HallSensorsGetPosition(void);
uint16_t PMSM_ADCToPWM(uint16_t ADC_VALUE);
void PMSM_SetPWM(uint16_t PWM);

void PMSM_HallSensorsInit(void);
void PMSM_PWMTimerInit(void);
void PMSM_SinTimerInit(void);
void PMSM_SpeedTimerInit(void);
void PMSM_SetPWM_UVW(uint16_t PWM1, uint16_t PWM2, uint16_t PWM3);
uint16_t PMSM_GetSpeed(void);
uint8_t PMSM_MotorSpeedIsOK(void);
uint8_t	PMSM_GetState(uint8_t index);

#endif
