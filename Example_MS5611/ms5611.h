#ifndef _MS5611_LIB_H_
#define _MS5611_LIB_H_

#define MS5611_addr	0xEE
#define MS5611_OSR	4 // 0..4
//OSR 0 - Resolution 0.065 mbar, Conversion time 0.60 ms
//OSR 1 - Resolution 0.042 mbar, Conversion time 1.17 ms
//OSR 2 - Resolution 0.027 mbar, Conversion time 2.28 ms
//OSR 3 - Resolution 0.018 mbar, Conversion time 4.54 ms
//OSR 4 - Resolution 0.012 mbar, Conversion time 9.04 ms

void MS5611_Init(void);
void MS5611_Convert(long* temperature, long* pressure);

#endif
