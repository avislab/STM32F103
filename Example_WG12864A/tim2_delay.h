#ifndef TIM2DELAY_LIB
#define TIM2DELAY_LIB

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "misc.h"

void TIM2_init(void);
void delay_us(uint32_t n_usec);
void delay_ms(uint32_t n_msec);

#endif
