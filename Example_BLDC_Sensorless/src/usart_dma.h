#ifndef _USART_DMA_LIB_H_
#define _USART_DMA_LIB_H_

void USART_DMA_Init(void);
void USART_DMA_Send(char *pucBuffer);
char USART_DMA_Ready(void);

#endif
