#include "stm32f10x.h"

static volatile uint32_t sysTickCount = 0;

void SysTick_Handler()
{
	if (sysTickCount != 0) {
		sysTickCount--;
	}
}

void sysTickDalayInit() {
  SysTick_Config(SystemCoreClock/1000);
}

void sysTickDalay(uint32_t nTime)
{
	sysTickCount = nTime;
	while(sysTickCount != 0);
}

void sysTickSet(uint32_t nTime)
{
	sysTickCount = nTime;
}

uint32_t sysTickGet()
{
	return sysTickCount;
}
