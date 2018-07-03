#ifndef SYSTICKDELAY_H_
#define SYSTICKDELAY_H_

void sysTickDalayInit();
void sysTickDalay(uint32_t nTime);
void sysTickSet(uint32_t nTime);
uint32_t sysTickGet();

#endif
