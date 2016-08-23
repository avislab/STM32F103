#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define IN1 GPIO_Pin_6
#define IN2 GPIO_Pin_7
#define IN3 GPIO_Pin_8
#define IN4 GPIO_Pin_9

uint16_t StepSequence[8] = {IN1, IN1|IN2, IN2, IN2|IN3, IN3, IN3|IN4, IN4, IN4|IN1};
uint8_t MotorStep = 0;

void StepMotorInit(void) {
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Enable PORTB Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	/* Configure the pins */
	GPIO_InitStructure.GPIO_Pin = IN1 | IN2 | IN3 | IN4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void StepMotorNextStep(void) {
	MotorStep++;
	if (MotorStep > 7)
		MotorStep = 0;

	GPIO_Write(GPIOB, StepSequence[MotorStep]);
}

void StepMotorPrevStep(void) {
	if (MotorStep > 0)
		MotorStep--;
	else
		MotorStep = 7;

	GPIO_Write(GPIOB, StepSequence[MotorStep]);
}

int main(void)
{
	int i;
	StepMotorInit();

    while(1)
    {
    	StepMotorNextStep();
    	/* delay */
    	for(i=0;i<0x10000;i++);
    }
}
