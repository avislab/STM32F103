#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#include "misc.h"

#include "i2c_slave.h"

/*******************************************************************/
extern volatile uint8_t ADCBuffer[16];

unsigned char i2c1_mode=I2C1_MODE_WAITING;

uint8_t i2c1_ram_adr=0;
uint8_t i2c1_ram[I2C1_RAM_SIZE+1];

/*******************************************************************/
uint8_t get_i2c1_ram(uint8_t adr) {
	//ADC data
	if ((ADC_ADDR_START <= adr) & (adr < ADC_ADDR_START + ADC_CHANNELS*2)) {
		return ADCBuffer[adr - ADC_ADDR_START];
	}
	else {
		// Other addresses
		return i2c1_ram[adr];
	}
}

void set_i2c1_ram(uint8_t adr, uint8_t val) {
	i2c1_ram[adr] = val;
	return;
}


/*******************************************************************/
void I2C1_Slave_init(void) {
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    I2C_InitTypeDef  I2C_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* Configure I2C_EE pins: SCL and SDA */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure the I2C event priority */
    NVIC_InitStructure.NVIC_IRQChannel                   = I2C1_EV_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //Configure I2C error interrupt to have the higher priority.
    NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    /* I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = I2CSLAVE_ADDR;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C1_CLOCK_FRQ;

    /* I2C Peripheral Enable */
    I2C_Cmd(I2C1, ENABLE);
    /* Apply I2C configuration after enabling it */
    I2C_Init(I2C1, &I2C_InitStructure);

    I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE); //Part of the STM32 I2C driver
    I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);
    I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE); //Part of the STM32 I2C driver
}
/*******************************************************************/

/*******************************************************************/
void I2C1_ClearFlag(void) {
  // ADDR-Flag clear
  while((I2C1->SR1 & I2C_SR1_ADDR) == I2C_SR1_ADDR) {
    I2C1->SR1;
    I2C1->SR2;
  }

  // STOPF Flag clear
  while((I2C1->SR1&I2C_SR1_STOPF) == I2C_SR1_STOPF) {
    I2C1->SR1;
    I2C1->CR1 |= 0x1;
  }
}
/*******************************************************************/

/*******************************************************************/
void I2C1_EV_IRQHandler(void) {
  uint32_t event;
  uint8_t wert;

  // Reading last event
  event=I2C_GetLastEvent(I2C1);

  if(event==I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED) {
    // Master has sent the slave address to send data to the slave
    i2c1_mode=I2C1_MODE_SLAVE_ADR_WR;
  }
  else if(event==I2C_EVENT_SLAVE_BYTE_RECEIVED) {
    // Master has sent a byte to the slave
    wert=I2C_ReceiveData(I2C1);
    // Check address
    if(i2c1_mode==I2C1_MODE_SLAVE_ADR_WR) {
      i2c1_mode=I2C1_MODE_ADR_BYTE;
      // Set current ram address
      i2c1_ram_adr=wert;
    }
    else {
      i2c1_mode=I2C1_MODE_DATA_BYTE_WR;
      // Store data in RAM
      set_i2c1_ram(i2c1_ram_adr, wert);
      // Next ram adress
      i2c1_ram_adr++;
    }
  }
  else if(event==I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED) {
    // Master has sent the slave address to read data from the slave
    i2c1_mode=I2C1_MODE_SLAVE_ADR_RD;
    // Read data from RAM
    wert=get_i2c1_ram(i2c1_ram_adr);
    // Send data to the master
    I2C_SendData(I2C1, wert);
    // Next ram adress
    i2c1_ram_adr++;
  }
  else if(event==I2C_EVENT_SLAVE_BYTE_TRANSMITTED) {
    // Master wants to read another byte of data from the slave
    i2c1_mode=I2C1_MODE_DATA_BYTE_RD;
    // Read data from RAM
    wert=get_i2c1_ram(i2c1_ram_adr);
    // Send data to the master
    I2C_SendData(I2C1, wert);
    // Next ram adress
    i2c1_ram_adr++;
  }
  else if(event==I2C_EVENT_SLAVE_STOP_DETECTED) {
    // Master has STOP sent
	I2C1_ClearFlag();
    i2c1_mode=I2C1_MODE_WAITING;
  }
}

/*******************************************************************/
void I2C1_ER_IRQHandler(void) {
  if (I2C_GetITStatus(I2C1, I2C_IT_AF)) {
    I2C_ClearITPendingBit(I2C1, I2C_IT_AF);
  }
}
/*******************************************************************/
