#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rtc.h"

unsigned char RTC_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)
	{
		RCC_BackupResetCmd(ENABLE);
		RCC_BackupResetCmd(DISABLE);

		RCC_LSEConfig(RCC_LSE_ON);
		while ((RCC->BDCR & RCC_BDCR_LSERDY) != RCC_BDCR_LSERDY) {}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

		/* Set prescaler */
		RTC_SetPrescaler(0x7FFF);

		/* Enable RTC */
		RCC_RTCCLKCmd(ENABLE);

		RTC_WaitForSynchro();

		return 1;
	}
	return 0;
}


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

  RTC_Init();
  /* Enable the RTC Alarm interrupt */
  RTC_ITConfig(RTC_IT_ALR, ENABLE);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  while (1)
  {
	  /* LED blink */
	  for(j=0; j<6; j++) {
		  /* Toggle LED which connected to PC13*/
		  GPIOC->ODR ^= GPIO_Pin_13;
		  /* delay */
		  for(i=0; i<0x100000; i++);
	  }

	  /* Alarm in 5 second */
	  RTC_SetAlarm(RTC_GetCounter()+ 5);
	  /* Wait until last write operation on RTC registers has finished */
	  RTC_WaitForLastTask();

	  /* Disable LED */
	  GPIO_SetBits(GPIOC, GPIO_Pin_13);

	  /* Enters STANDBY mode */
	  PWR_EnterSTANDBYMode();
  }
}
