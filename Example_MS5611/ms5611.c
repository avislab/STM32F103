#include "stm32f10x_i2c.h"
#include "ms5611.h"
#include "math.h"

unsigned short ms5611_C1, ms5611_C2, ms5611_C3, ms5611_C4, ms5611_C5, ms5611_C6;

void delay_us(uint32_t n_usec);
void delay_ms(uint32_t n_usec);

short ms5611ReadShort(unsigned char address)
{
  unsigned short msb=0;
  unsigned short lsb=0;

  I2C_AcknowledgeConfig(I2C1,ENABLE);
  I2C_GenerateSTART(I2C1,ENABLE);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
  I2C_Send7bitAddress(I2C1, MS5611_addr, I2C_Direction_Transmitter);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1,address);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, MS5611_addr, I2C_Direction_Receiver);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  msb = I2C_ReceiveData(I2C1);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  lsb = I2C_ReceiveData(I2C1);

  I2C_GenerateSTOP(I2C1,ENABLE);
  I2C_AcknowledgeConfig(I2C1,DISABLE);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  I2C_ReceiveData(I2C1);

  return (msb << 8) | lsb;
}

unsigned long ms5611ReadLong(unsigned char address)
{
  unsigned long result=0;

  unsigned long msb=0;
  unsigned long lsb=0;
  unsigned long xsb=0;

  I2C_AcknowledgeConfig(I2C1,ENABLE);
  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, MS5611_addr, I2C_Direction_Transmitter);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1,address);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, MS5611_addr, I2C_Direction_Receiver);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  msb = I2C_ReceiveData(I2C1);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  lsb = I2C_ReceiveData(I2C1);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  xsb = I2C_ReceiveData(I2C1);

  I2C_GenerateSTOP(I2C1,ENABLE);
  I2C_AcknowledgeConfig(I2C1,DISABLE);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  I2C_ReceiveData(I2C1);

  result = (msb << 16) | (lsb << 8) | xsb;

  return result;
}

//void ms5611WriteByte(unsigned char address, unsigned char data)
void ms5611WriteByte(unsigned char data)
{

  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, MS5611_addr, I2C_Direction_Transmitter);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

//  I2C_SendData(I2C1,address);
//  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_SendData(I2C1,data);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTOP(I2C1,ENABLE);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}
//----------------------------------------

void MS5611_Init(void) {
	// Reset
	ms5611WriteByte(0x1E);

	delay_ms(1);

	// Read calibration data
	ms5611_C1 = ms5611ReadShort(0xA2);
	ms5611_C2 = ms5611ReadShort(0xA4);
	ms5611_C3 = ms5611ReadShort(0xA6);
	ms5611_C4 = ms5611ReadShort(0xA8);
	ms5611_C5 = ms5611ReadShort(0xAA);
	ms5611_C6 = ms5611ReadShort(0xAC);
}

void MS5611_Convert(long* temperature, long* pressure) {

	unsigned long D1, D2;
	long dT, TEMP;
	long long OFF, SENS, OFF2, SENS2, T2;

	// Start Pressure conversion
	ms5611WriteByte(0x40 + MS5611_OSR * 2);
	delay_ms(1 + MS5611_OSR * 2);
	// Read Pressure data
	D1 = ms5611ReadLong(0x00);

	// Start Temperature conversion
	ms5611WriteByte(0x50 + MS5611_OSR * 2);
	delay_ms(1 + MS5611_OSR * 2);
	// Read Temperature data
	D2 = ms5611ReadLong(0x00);

	dT = D2 - (ms5611_C5 << 8);
	TEMP = 2000 + (((long long)dT * (long long)ms5611_C6) >> 23);
	OFF = ((long long)ms5611_C2 << 16) + (((long long)ms5611_C4 * (long long)dT) >> 7);
	SENS = ((long long)ms5611_C1 << 15 ) + (((long long)ms5611_C3 * (long long)dT ) >> 8);

	if (TEMP >= 2000) {
		T2 = 0;
		OFF2 = 0;
		SENS2 = 0;
	}
	else if (TEMP < 2000) {
		T2 = (((long long)dT * (long long)dT) >> 31);
		OFF2 = 5 * (((long long)TEMP - 2000) * ((long long)TEMP - 2000)) >> 1;
		SENS2 = 5 * (((long long)TEMP - 2000) * ((long long)TEMP - 2000)) >> 2;
		if (TEMP < -1500 ) {
			OFF2 = OFF2 + 7 * (((long long)TEMP + 1500) * ((long long)TEMP + 1500));
			SENS2 = SENS2 + ((11 *(((long long)TEMP + 1500) * ((long long)TEMP + 1500))) >> 1);
		}
	}

	TEMP = TEMP - T2;
	OFF = OFF - OFF2;
	SENS = SENS - SENS2;

	*pressure = (unsigned long) (((((D1 * SENS) >> 21) - OFF)) >> 15);
	*temperature = (long)TEMP/10;
}
//----------------------------------------


