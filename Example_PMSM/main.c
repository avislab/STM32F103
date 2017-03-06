#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "misc.h"
#include "sysclk.h"
#include "adc_dma.h"
#include "pmsm.h"

int main(void)
{
	SetSysClockTo72();

	// ADC Init
	ADC_DMA_init();

	// PMSM Init
	PMSM_Init();

	// Reverse pin Init
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_0);

    while(1)
    {
    	if (ADCBuffer[0] > PMSM_ADC_START) {
    		// If Motor Is not run
    		if (PMSM_MotorIsRun() == 0) {
    			// Start motor
    			// Check Reverse pin
    			if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) != 0) {
    				// Forward
    				PMSM_MotorSetSpin(PMSM_CW);
    			}
    			else {
    				// Backward
    				PMSM_MotorSetSpin(PMSM_CCW);
    			}
    			PMSM_MotorCommutation(PMSM_HallSensorsGetPosition());
    			PMSM_MotorSetRun();
    		}
   			PMSM_SetPWM(PMSM_ADCToPWM(ADCBuffer[0]));
    	}
    	else {
    		PMSM_SetPWM(0);
    	}
    }
}
