#include "stm32f10x.h"

#include "sysclk.h"
#include "systickdelay.h"

int main(void)
{
	uint32_t i;

	// Set clock
	SetSysClockTo72();

	// SysTick Init
	sysTickDalayInit();

	GPIO_InitTypeDef  GPIO_InitStructure;

	/* Initialize LED which connected to PC13 */
	// Enable PORTC Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Toggle LED which connected to PC13*/
	for (i = 0; i < 10; i++) {
		GPIOC->ODR ^= GPIO_Pin_13;
    	sysTickDalay(500); // Delay & wait
	}

	while (1) {
		// do something
		// ..

		if (sysTickGet() == 0) {
			sysTickSet(1000);
			GPIOC->ODR ^= GPIO_Pin_13;
		}
	}
}
