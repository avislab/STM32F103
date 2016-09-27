#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_wwdg.h"
#include "misc.h"

void WWDG_IRQHandler(void) {
	WWDG_ClearFlag(); //This function reset flag WWDG->SR and cancel the resetting
	WWDG_SetCounter(100);

	/* Toggle LED which connected to PC13*/
    GPIOC->ODR ^= GPIO_Pin_13;
}

int main(void)
{
  /* Initialize Leds mounted on STM32 board */
  GPIO_InitTypeDef  GPIO_InitStructure;
  /* Initialize LED which connected to PC13, Enable the Clock*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Disable LED */
  GPIO_SetBits(GPIOC, GPIO_Pin_13);

  /* Enable Watchdog*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG,ENABLE);
  WWDG_DeInit();
  WWDG_SetPrescaler(WWDG_Prescaler_8); //1, 2, 4, 8
  WWDG_SetWindowValue(127); // 64...127
  WWDG_Enable(100);
  WWDG_EnableIT();

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;    /*WWDG interrupt*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);/*    NVIC initialization*/

  while (1)
  {

  }
}
