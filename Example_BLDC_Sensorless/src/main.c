#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

#include "sysclk.h"
#include "systickdelay.h"

#include "adc_dma.h"
#include "bldc.h"

int main(void)
{
	uint16_t ADC_POT = 0;

	// Set clock
	SetSysClockTo72();

	sysTickDalayInit();

	// ADC Init
	ADC_DMA_init();
	// TIM1, TIM3, TIM4, outputs, inputs, interrupts, etc. Init
	BLDC_Init();

    while(1)
    {
    	// ======== Damper =========
    	/*
    	sysTickDalay(1);
		if (ADCBuffer[0] > ADC_POT) {
			if (ADC_POT < 4096) {
				ADC_POT++;
			}
		}
		else {
			if (ADC_POT > 0) {
				ADC_POT--;
			}
		}
		*/
    	ADC_POT = ADCBuffer[0];
		// =========================

		if (ADC_POT > BLDC_ADC_START) { // Start
    		if (BLDC_MotorGetSpin() == BLDC_STOP) { // If motor is stopped
    			BLDC_MotorSetSpin(BLDC_START); // Set START flag
    			ADC_POT = BLDC_ADC_MOTOR_START; // Set start PWM value
    			BLDC_MotorStart(); // try to start motor
    		}
    		else {
   				BLDC_SetPWM(BLDC_ADCToPWM(ADC_POT));
    		}
    	}
    	else {
    		if (BLDC_MotorGetSpin() != BLDC_STOP) {
    			if (ADC_POT < BLDC_ADC_STOP) { // if need to stop motor
    				BLDC_SetPWM(BLDC_ADC_ZERO); // set minimum PWM just for rotor position detection
    				//BLDC_MotorStop();
    			}
    			else {
    				BLDC_SetPWM(BLDC_ADCToPWM(ADC_POT));
    			}
    		}
    	}
    }
}
