#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_usart.h"
#include "stdio.h"
#include "misc.h"

void usart_init(void)
{
	    /* Enable USART1 and GPIOA clock */
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	    /* Configure the GPIOs */
	    GPIO_InitTypeDef GPIO_InitStructure;

	    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);

	    /* Configure USART1 Rx (PA.10) as input floating */
	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);

	    /* Configure the USART1 */
	    USART_InitTypeDef USART_InitStructure;

	  /* USART1 configuration ------------------------------------------------------*/
	    /* USART1 configured as follow:
	          - BaudRate = 115200 baud
	          - Word Length = 8 Bits
	          - One Stop Bit
	          - No parity
	          - Hardware flow control disabled (RTS and CTS signals)
	          - Receive and transmit enabled
	          - USART Clock disabled
	          - USART CPOL: Clock is active low
	          - USART CPHA: Data is captured on the middle
	          - USART LastBit: The clock pulse of the last data bit is not output to
	                           the SCLK pin
	    */
	    USART_InitStructure.USART_BaudRate = 115200;
	    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	    USART_InitStructure.USART_StopBits = USART_StopBits_1;
	    USART_InitStructure.USART_Parity = USART_Parity_No;
	    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	    USART_Init(USART1, &USART_InitStructure);

	    /* Enable USART1 */
	    USART_Cmd(USART1, ENABLE);
}

void USARTSend(char *pucBuffer)
{
    while (*pucBuffer)
    {
        USART_SendData(USART1, *pucBuffer++);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {
        }
    }
}

//========================================================================================
unsigned char RTC_Init(void)
{
	// Дозволити тактування модулів управління живленням і управлінням резервної областю
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	// Дозволити доступ до області резервних даних
	PWR_BackupAccessCmd(ENABLE);
	// Якщо годинник вимкнений - ініціалізувати
	if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)
	{
		// Виконати скидання області резервних даних
		RCC_BackupResetCmd(ENABLE);
		RCC_BackupResetCmd(DISABLE);

		// Вибрати джерелом тактових імпульсів зовнішній кварц 32768 і подати тактування
		RCC_LSEConfig(RCC_LSE_ON);
		while ((RCC->BDCR & RCC_BDCR_LSERDY) != RCC_BDCR_LSERDY) {}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

		RTC_SetPrescaler(0x7FFF); // Встановити поділювач щоб годинник рахував секунди

		// Вмикаємо годинник
		RCC_RTCCLKCmd(ENABLE);

		// Чекаємо на синхронізацію
		RTC_WaitForSynchro();

		return 1;
	}
	return 0;
}

/*
* Як конвертувати дату можна прочитати тут:
* https://ru.m.wikipedia.org/wiki/%D0%AE%D0%BB%D0%B8%D0%B0%D0%BD%D1%81%D0%BA%D0%B0%D1%8F_%D0%B4%D0%B0%D1%82%D0%B0
*/

// (UnixTime = 00:00:00 01.01.1970 = JD0 = 2440588)
#define JULIAN_DATE_BASE	2440588

typedef struct
{
	uint8_t RTC_Hours;
	uint8_t RTC_Minutes;
	uint8_t RTC_Seconds;
	uint8_t RTC_Date;
	uint8_t RTC_Month;
	uint16_t RTC_Year;
} RTC_DateTimeTypeDef;

// Convert Counter to Date
void RTC_GetDateTime(uint32_t RTC_Counter, RTC_DateTimeTypeDef* RTC_DateTimeuct)
{
	unsigned long time;
	unsigned long t1, a, b, c, d, e, m;
	int year = 0;
	int mon = 0;
	//int wday = 0;
	int mday = 0;
	int hour = 0;
	int min = 0;
	int sec = 0;
	uint64_t jd = 0;;
	uint64_t jdn = 0;

	jd = ((RTC_Counter+43200)/(86400>>1)) + (2440587<<1) + 1;
	jdn = jd>>1;

	time = RTC_Counter;
	t1 = time/60;
	sec = time - t1*60;

	time = t1;
	t1 = time/60;
	min = time - t1*60;

	time = t1;
	t1 = time/24;
	hour = time - t1*24;

	//wday = jdn%7;

	a = jdn + 32044;
	b = (4*a+3)/146097;
	c = a - (146097*b)/4;
	d = (4*c+3)/1461;
	e = c - (1461*d)/4;
	m = (5*e+2)/153;
	mday = e - (153*m+2)/5 + 1;
	mon = m + 3 - 12*(m/10);
	year = 100*b + d - 4800 + (m/10);

	RTC_DateTimeuct->RTC_Year = year;
	RTC_DateTimeuct->RTC_Month = mon;
	RTC_DateTimeuct->RTC_Date = mday;
	RTC_DateTimeuct->RTC_Hours = hour;
	RTC_DateTimeuct->RTC_Minutes = min;
	RTC_DateTimeuct->RTC_Seconds = sec;
	//RTC_DateTimeuct->dow = wday;
}

// Convert Date to Counter
uint32_t RTC_GetRTC_Counter(RTC_DateTimeTypeDef* RTC_DateTimeuct) {
	uint8_t a;
	uint16_t y;
	uint8_t m;
	uint32_t JDN;

	a=(14-RTC_DateTimeuct->RTC_Month)/12;
	y=RTC_DateTimeuct->RTC_Year+4800-a;
	m=RTC_DateTimeuct->RTC_Month+(12*a)-3;

	JDN=RTC_DateTimeuct->RTC_Date;
	JDN+=(153*m+2)/5;
	JDN+=365*y;
	JDN+=y/4;
	JDN+=-y/100;
	JDN+=y/400;
	JDN = JDN -32045;
	JDN = JDN - JULIAN_DATE_BASE;
	JDN*=86400;
	JDN+=(RTC_DateTimeuct->RTC_Hours*3600);
	JDN+=(RTC_DateTimeuct->RTC_Minutes*60);
	JDN+=(RTC_DateTimeuct->RTC_Seconds);

	return JDN;
}
//========================================================================================

int main(void)
{
	char buffer[80] = {'\0'};
	uint32_t RTC_Counter = 0;
	RTC_DateTimeTypeDef	RTC_DateTime;

	usart_init();
	if (RTC_Init() == 1) {
		// Якщо перша ініціалізіція RTC Встановлюємо початкову дату, наприклад 14.10.2016 14:35:17
		RTC_DateTime.RTC_Date = 14;
		RTC_DateTime.RTC_Month = 10;
		RTC_DateTime.RTC_Year = 10;

		RTC_DateTime.RTC_Hours = 14;
		RTC_DateTime.RTC_Minutes = 35;
		RTC_DateTime.RTC_Seconds = 17;

		RTC_SetCounter(RTC_GetRTC_Counter(&RTC_DateTime));
	}

    while(1)
    {
    	RTC_Counter = RTC_GetCounter();
    	sprintf(buffer, "COUNTER: %d\r\n", (int)RTC_Counter);
		USARTSend(buffer);

		RTC_GetDateTime(RTC_Counter, &RTC_DateTime);
		sprintf(buffer, "%02d.%02d.%04d  %02d:%02d:%02d\r\n",
				RTC_DateTime.RTC_Date, RTC_DateTime.RTC_Month, RTC_DateTime.RTC_Year,
				RTC_DateTime.RTC_Hours, RTC_DateTime.RTC_Minutes, RTC_DateTime.RTC_Seconds);

		/* delay */
		while (RTC_Counter == RTC_GetCounter()) {

		}

    }
}
