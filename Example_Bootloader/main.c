#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "mass_mal.h"

#define BUTTON_PIN	GPIO_Pin_1

void GoToUserApp(void)
{
	u32 appJumpAddress;
	void (*GoToApp)(void);

	appJumpAddress = *((volatile u32*)(FLASH_DISK_START_ADDRESS + 4));
	GoToApp = (void (*)(void))appJumpAddress;
	SCB->VTOR = FLASH_DISK_START_ADDRESS;
	__set_MSP(*((volatile u32*) FLASH_DISK_START_ADDRESS)); //stack pointer (to RAM) for USER app in this address
	GoToApp();
}

int main(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* Initialize Button input PB */
	// Enable PORTB Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	/* Configure the GPIO_BUTTON pin */
	GPIO_InitStructure.GPIO_Pin = BUTTON_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	if (GPIO_ReadInputDataBit(GPIOB, BUTTON_PIN) == Bit_SET) {
		// Go To Application
		GPIO_DeInit(GPIOB);
		GoToUserApp();
	}
	else {
		// Initialize Mass Storage
		Set_System();
		Set_USBClock();
		USB_Interrupts_Config();
		USB_Init();
		while (bDeviceState != CONFIGURED);

		while (1)
		{

		}
	}
}

