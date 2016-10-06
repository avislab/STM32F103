#include "stm32f10x_i2c.h"
#include "bmp280.h"
#include <math.h>

unsigned short dig_T1;
short dig_T2;
short dig_T3;
unsigned short dig_P1;

short dig_P2;
short dig_P3;
short dig_P4;
short dig_P5;
short dig_P6;
short dig_P7;
short dig_P8;
short dig_P9;

short bmp280ReadShort(unsigned char address)
{
  short msb=0;
  short lsb=0;

  I2C_AcknowledgeConfig(I2C1,ENABLE);
  I2C_GenerateSTART(I2C1,ENABLE);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Transmitter);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1,address);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Receiver);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  lsb = I2C_ReceiveData(I2C1);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  msb = I2C_ReceiveData(I2C1);

  I2C_GenerateSTOP(I2C1,ENABLE);
  I2C_AcknowledgeConfig(I2C1,DISABLE);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  I2C_ReceiveData(I2C1);

  return (msb << 8) | lsb;
}

unsigned long bmp280ReadLong(unsigned char address)
{
  unsigned long result=0;

  unsigned long msb=0;
  unsigned long lsb=0;
  unsigned long xsb=0;

  I2C_AcknowledgeConfig(I2C1,ENABLE);
  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Transmitter);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1,address);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Receiver);
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
  
  return (result >> 4);
}

void bmp280WriteByte(unsigned char address, unsigned char data)
{
  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Transmitter);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1,address);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_SendData(I2C1,data);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTOP(I2C1,ENABLE);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}
//----------------------------------------

void bmp280Convert(long* temperature, long* pressure)
{
  unsigned long adc_T;
  unsigned long adc_P;
  adc_T = bmp280ReadLong(BMP280_REG_RESULT_TEMPRERATURE);
  adc_P = bmp280ReadLong(BMP280_REG_RESULT_PRESSURE);

  double var1, var2, p, t_fine;
  var1 = (((double)adc_T)/16384.0 - ((double)dig_T1)/1024.0) * ((double)dig_T2);
  var2 = ((((double)adc_T)/131072.0 - ((double)dig_T1)/8192.0) * (((double)adc_T)/131072.0 - ((double) dig_T1)/8192.0)) * ((double)dig_T3);
  t_fine = (var1 + var2);
  *temperature = (int32_t) (t_fine*10 / 5120.0);

  var1 = ((double)t_fine/2.0) - 64000.0;
  var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
  var2 = var2 + var1 * ((double)dig_P5) * 2.0;
  var2 = (var2/4.0)+(((double)dig_P4) * 65536.0);
  var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
  var1 = (1.0 + var1 / 32768.0)*((double)dig_P1);
  if (var1 == 0.0)
  {
    return; // avoid exception caused by division by zero
  }
  p = 1048576.0 - (double)adc_P;
  p = (p - (var2 / 4096.0)) * 6250.0 / var1;
  var1 = ((double)dig_P9) * p * p / 2147483648.0;
  var2 = p * ((double)dig_P8) / 32768.0;
  p = (p + (var1 + var2 + ((double)dig_P7)) / 16.0);

  *pressure = (long)p;
}
//----------------------------------------

void BMP280_Init(void)
{ 
  dig_T1 = bmp280ReadShort(0x88);//dig_T1
  dig_T2 = bmp280ReadShort(0x8A);//dig_T2
  dig_T3 = bmp280ReadShort(0x8C);//dig_T3
  dig_P1 = bmp280ReadShort(0x8E);//dig_P1
  dig_P2 = bmp280ReadShort(0x90);//dig_P2
  dig_P3 = bmp280ReadShort(0x92);//dig_P3
  dig_P4 = bmp280ReadShort(0x94);//dig_P4
  dig_P5 = bmp280ReadShort(0x96);//dig_P5
  dig_P6 = bmp280ReadShort(0x98);//dig_P6
  dig_P7 = bmp280ReadShort(0x9A);//dig_P7
  dig_P8 = bmp280ReadShort(0x9C);//dig_P8
  dig_P9 = bmp280ReadShort(0x9E);//dig_P9

  bmp280WriteByte(BMP280_REG_CONFIG, BMP280_CONFIG);
  bmp280WriteByte(BMP280_REG_CONTROL, BMP280_MEAS);
}

//----------------------------------------

