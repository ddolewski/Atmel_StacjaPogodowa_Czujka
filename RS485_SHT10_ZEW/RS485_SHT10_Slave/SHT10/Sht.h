//----------------------------------------------------------------------------------
//
// Sensirion SHT1x Humidity Sensor Library
//
//----------------------------------------------------------------------------------

#ifndef __sht_h
#define __sht_h

#define SHT_TEMPERATURE 	0x03		// Measure temp - for ShtMeasure
#define SHT_HUMIDITY 		0x05		// Measure humidity - for ShtMeasure

#define SHT_DDR				DDRD		// Port with clock and data pins
#define SHT_PORT			PORTD		// Port with clock and data pins
#define SHT_PIN				PIND		// Port with clock and data pins
#define SHT_CLOCK			6			// Pin used to output clock to SHT
#define SHT_DATA			5			// Pin used to read/output data from/to SHT

#define STATUS_REG_W 		0x06 		// Command to read status register
#define STATUS_REG_R 		0x07 		// Command to write status register
#define RESET 				0x1E 		// Command for soft reset (not currently used)

void SHT_Init(void);
void SHT_Reset(void);
int  SHT_Measure(unsigned char mode);
void SHT_Calculate(int *p_temperature, int *p_humidity);
unsigned char SHT_ReadStatusReg(unsigned char * error);
unsigned char SHT_WriteStatusReg(unsigned char value);
//void SHT_GetResults(void);
void SHT_GetResults(float * shtRealTemp, int * shtRealHum);
unsigned char SHT_SoftReset(void);
#endif
