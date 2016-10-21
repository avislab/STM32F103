/*******************************************************************/
#define I2CSLAVE_ADDR	0x4E //0x27
#define ADC_ADDR_START		0x00
#define ADC_CHANNELS		8
// 0,1 - ADC0
// 2,3 - ADC1
// 4,5 - ADC2
// 6,7 - ADC3
// 8,9 - ADC4
// 10,11 - ADC5
// 12,13 - ADC6
// 14,15 - ADC7

/*******************************************************************/
#define   I2C1_CLOCK_FRQ   100000    // I2C-Frq in Hz (100 kHz)
#define   I2C1_RAM_SIZE    0xFF      // RAM Size in Byte (1...255)
									 // First 8 bytes is a ADC Data,
									 //   next addresses You can use for
									 //   reading and writing.

#define I2C1_MODE_WAITING		0    // Waiting for commands
#define I2C1_MODE_SLAVE_ADR_WR	1	 // Received slave address (writing)
#define I2C1_MODE_ADR_BYTE		2    // Received ADR byte
#define I2C1_MODE_DATA_BYTE_WR	3    // Data byte (writing)
#define I2C1_MODE_SLAVE_ADR_RD	4 	 // Received slave address (to read)
#define I2C1_MODE_DATA_BYTE_RD	5    // Data byte (to read)
/*******************************************************************/

void I2C1_Slave_init(void);

/*******************************************************************/
