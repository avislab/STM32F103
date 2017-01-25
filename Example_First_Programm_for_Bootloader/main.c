#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

int main(void)
{
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x08002800);

  int i;
  /* Initialize Leds mounted on STM32 board */
  GPIO_InitTypeDef  GPIO_InitStructure;
  /* Initialize LED which connected to PC13, Enable the Clock*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  while (1)
  {
    /* Toggle LED which connected to PC13*/
    GPIOC->ODR ^= GPIO_Pin_13;

    /* delay */
    for(i=0;i<0x100000;i++);
  }
}
