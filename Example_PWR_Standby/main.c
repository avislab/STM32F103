#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"

int main(void)
{
  int i, j;
  /* Initialize Leds mounted on STM32 board */
  GPIO_InitTypeDef  GPIO_InitStructure;
  /* Initialize LED which connected to PC13, Enable the Clock*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // ENABLE Wake Up Pin
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
  PWR_WakeUpPinCmd(ENABLE);

  while (1)
  {
	  /* LED blink */
	  for(j=0; j<6; j++) {
		  /* Toggle LED which connected to PC13*/
		  GPIOC->ODR ^= GPIO_Pin_13;
		  /* delay */
		  for(i=0; i<0x100000; i++);
	  }

	  /* Disable LED */
	  GPIO_SetBits(GPIOC, GPIO_Pin_13);
	  /* Enters STANDBY mode */
	  PWR_EnterSTANDBYMode();
  }
}
