#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"

int main(void)
{
	Set_System();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();
	while (bDeviceState != CONFIGURED);

	while (1)
	{

	}
}
