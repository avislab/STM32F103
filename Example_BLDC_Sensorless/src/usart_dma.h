#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"

char usart_buffer[120] = {'\0'};

void usart_dma_init(void)
{
	/* Enable USART1 and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* DMA */
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&usart_buffer[0];
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStruct.DMA_BufferSize = sizeof(usart_buffer);
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Low;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel7, &DMA_InitStruct);

	/* NVIC Configuration */
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the GPIOs */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
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
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	//USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;

	USART_Init(USART2, &USART_InitStructure);

	/* Enable USART1 */
	USART_Cmd(USART2, ENABLE);

	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	//DMA_Cmd(DMA1_Channel4, ENABLE);

	DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);


	/* Enable the USART1 Receive interrupt: this interrupt is generated when the
	USART1 receive data register is not empty */
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void USARTSendDMA(void)
{
	//strcpy(usart_buffer, pucBuffer);

	/* Restart DMA Channel*/
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA1_Channel7->CNDTR = strlen(usart_buffer);
	DMA_Cmd(DMA1_Channel7, ENABLE);
}

/*
void USARTSendDMA(const unsigned char *pucBuffer)
{
	strcpy(usart_buffer, pucBuffer);

	// Restart DMA Channel
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA1_Channel4->CNDTR = strlen(pucBuffer);
	DMA_Cmd(DMA1_Channel4, ENABLE);
}
*/

void DMA1_Channel7_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA1_IT_TC7);
	DMA_Cmd(DMA1_Channel7, DISABLE);
}
