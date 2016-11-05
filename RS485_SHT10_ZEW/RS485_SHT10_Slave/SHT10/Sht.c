//----------------------------------------------------------------------------------
//
// Sensirion SHTxx Humidity Sensor Library
//
// Library for using the SHT1x humidity and temperature
// sensors from Sensirion (http://www.sensirion.com).
// Based on Sensirion application note "Sample Code SHTxx".
//
// To use:
// - supply 5V to SHTxx (constants in calculations assume 5V, see ShtCalculate() and SHTxx datasheet)
// - connect the clock pin of the SHTxx to pin B1 (see Sht.h to change)
// - connect the data pin of the SHTxx to pin B2 (see Sht.h to change)
//
// - call ShtInit() to initialize pins, call when the processor starts
// - call ShtMeasure(MEASURE_TEMP) and ShtMeasure(MEASURE_HUMIDITY) to make the measurements
// - call ShtCalculate() to convert measurements to real-world units
//
// - call ShtReadStatus() and ShtWriteStatus() to modify the status register
//
//
// ToDo:
// - verify checksum digits sent from SHTxx
// - implement soft-reset
// - handle 12/8-bit temp/humidity readings
//
// History:
// 2003-Jul-03	BL	- Created
//
//----------------------------------------------------------------------------------

#include <avr/io.h>
#include <util/delay.h>
#include "sht.h"

void SHT_EnableData(void) 		
{ 
	SHT_DDR |= (1 << SHT_DATA); // Drive DATA pin
}			
void SHT_DisableData(void)		
{ 
	SHT_DDR &= ~ (1 << SHT_DATA); 		// Release DATA pin
	SHT_PORT |=  (1 << SHT_DATA); 
}
void SHT_DataHigh(void)		
{ 
	SHT_PORT |= (1 << SHT_DATA); // DATA pin high
}		
void SHT_DataLow(void)			
{ 
	SHT_PORT &= ~(1 << SHT_DATA);  // DATA pin low
}		
void SHT_ClockHigh(void)		
{ 
	SHT_PORT |= (1 << SHT_CLOCK); // CLOCK pin high 
} 		
void SHT_ClockLow(void)		
{ 
	SHT_PORT &= ~(1 << SHT_CLOCK); // CLOCK pin low
}	

//----------------------------------------------------------------------------------
// Initialize AVR i/o pins.
//----------------------------------------------------------------------------------
void SHT_Init(void)
{
	SHT_DDR |= (1 << SHT_CLOCK);		// Set clock pin to output
	SHT_PORT &= ~ (1 << SHT_CLOCK);		// Turn off clock pin
	SHT_DisableData();
	_delay_ms(50);
	SHT_WriteStatusReg(0x00);
}

//----------------------------------------------------------------------------------
// generates a transmission start
//       _____         ________
// DATA:      |_______|
//           ___     ___
// SCK : ___|   |___|   |______
//
//----------------------------------------------------------------------------------
void SHT_TransmissionStart(void)
{
	SHT_EnableData();
	SHT_DataHigh();
	SHT_ClockLow();
	_delay_us(5);
	_delay_us(5);
	SHT_ClockHigh();	
	_delay_us(5);
	SHT_DataLow();		
	_delay_us(5);
	SHT_ClockLow();	
	_delay_us(10); 
	SHT_ClockHigh();	
	_delay_us(5);
	SHT_DataHigh();	
	_delay_us(5);
	SHT_ClockLow();
}

//----------------------------------------------------------------------------------
// communication reset: DATA-line=1 and at least 9 SCK cycles followed by transstart
//       _____________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
//----------------------------------------------------------------------------------
void SHT_Reset(void)
{
	unsigned char i;
	SHT_EnableData();
	SHT_DataHigh();
	SHT_ClockLow();
	for(i=0;i<10;i++) //9 SCK cycles
	{
		SHT_ClockHigh();
	}
	
	SHT_TransmissionStart(); //transmission start
}

//----------------------------------------------------------------------------------
// reads a byte form the Sensibus and gives an acknowledge in case of "ack=1"
//----------------------------------------------------------------------------------
unsigned char SHT_ReadByte(void)
{
	unsigned char i,val=0;
	SHT_DisableData();

	// Read in 8 bits, LSB first
	for (i=0x80;i>0;i/=2)
	{
		SHT_ClockHigh();
		_delay_us(5);
		if (bit_is_set(SHT_PIN, SHT_DATA))
			val = (val | i); //read bit
		SHT_ClockLow();
	}

	// Send ACK
	SHT_EnableData();
	SHT_DataLow();
	SHT_ClockHigh();
	_delay_us(5);
	SHT_ClockLow();
	SHT_DisableData();

	return val;
}

//----------------------------------------------------------------------------------
// Writes a byte on the Sensibus and checks the acknowledge.
// Returns 0 if the successful
//----------------------------------------------------------------------------------
unsigned char SHT_WriteByte(unsigned char value)
{
	unsigned char i;
	unsigned char error = 0;

	// Write each bit one at a time, LSB first
	SHT_EnableData();
	for (i=0x80;i>0;i/=2)
	{
		if (i & value)
			SHT_DataHigh();
		else
			SHT_DataLow();

		SHT_ClockHigh();
		_delay_us(5);
		_delay_us(5);
		SHT_ClockLow();

	}
	SHT_DisableData();

	// Read ACK
	SHT_ClockHigh();
	_delay_us(5);
	error = bit_is_set(SHT_PIN, SHT_DATA);
	SHT_ClockLow();

	return error; //error=1 in case of no acknowledge
}

//----------------------------------------------------------------------------------
// Read humidity or temperature from the sensor.
// Returns the value in ticks. Use sht_calc() to convert to real world units.
// Returns 0xFFFF if the measurement failed
//----------------------------------------------------------------------------------
int SHT_Measure(unsigned char mode)
{
	unsigned int 	temp = 0xFFFF;
	unsigned char 	c;

	// Signal start of communications
	SHT_TransmissionStart();
	// Request measurement
	SHT_WriteByte(mode);
	// Sensor lowers the data line when measurement
	// is complete. Wait up to 2 seconds for this.
	for (c=0; c<20; c++)
	{
		if (! bit_is_set(SHT_PIN, SHT_DATA))
			break;
		_delay_ms(15);
	}
	// Read the measurement
	if (! bit_is_set(SHT_PIN, SHT_DATA))
	{
		temp = SHT_ReadByte();
		temp = temp << 8;
		temp += SHT_ReadByte();
	}
	return temp;
}

//----------------------------------------------------------------------------------------
// Calculates tempurature in ^C and humidity in %RH (temperature compensated)
// sht_measure() returns temp and humidity in ticks. Use this function to convert
// to compensated values in real world units.
//
// This function returns integers with 2 assumed decimal places. For example 2550
// means 25.50. This is to avoid including the floating point math library.
//
// input :	humi [Ticks] (12 bit)
// 			temp [Ticks] (14 bit)
// output: 	humi [%RH] (2 fixed decimals)
// 			temp [°C]  (2 fixed decimals)
//----------------------------------------------------------------------------------------
void SHT_Calculate(int *p_temperature, int *p_humidity)
{
	const long D1x100 = -40 * 100;					// for 5V power
	const long D2x100 = 0.01 * 100;					// for 14bit temp
	const long C1x100 = -4 * 100;					// for 12bit humidity
	const long C2x10000 = 0.0405 * 10000;			// for 12bit humidity
	const long C3x10000000 = -0.0000028 * 10000000;	// for 12bit humidity
	const long T1x100000 = 0.01 * 100000;			// for 12bit humidity
	const long T2x100000 = 0.00008 * 100000;		// for 12bit humidity

	long t = *p_temperature;			// temperatere in ticks from sensor
	long rh = *p_humidity;				// humidity in ticks from sensor

	long t_C;							// temperature in celcius: 2 fixed decimals
	long rh_lin;						// relative humidity: 2 fixed decimals
	long rh_true;						// temp compensated humidity: 2 fixed decimals

	t_C = D1x100 + D2x100*t;			// calculate tempurature in celcius from ticks

	rh_lin = (C3x10000000*rh*rh)/100000 + (C2x10000*rh)/100 + C1x100;
	rh_true = ((t_C-(25*100)) * (T1x100000 + T2x100000*rh))/100000 + rh_lin;

	if(rh_true > 10000)
		rh_true = 10000; 	//cut if the value is outside of
	if(rh_true < 10)
		rh_true = 10; 			//the physical possible range

	*p_temperature = (int)t_C; 			//return temperature [^C]
	*p_humidity = (int)rh_true;	 		//return humidity[%RH]
}
//----------------------------------------------------------------------------------
// Reads the status register 
//----------------------------------------------------------------------------------
unsigned char SHT_ReadStatusReg(unsigned char * error)
{
	unsigned char data = 0 ,errorsht = 0;
	SHT_TransmissionStart(); 					//transmission start
	errorsht = SHT_WriteByte(STATUS_REG_R); 	//send command to sensor
	*error = errorsht;
	data = SHT_ReadByte(); 			//read status register (8-bit)
	return data; 						//error=1 in case of no response form the sensor
}
//----------------------------------------------------------------------------------
// Writes the status register . Note this library only supports the default
// 14 bit temp and 12 bit humidity readings.
//----------------------------------------------------------------------------------
unsigned char SHT_WriteStatusReg(unsigned char value)
{
	unsigned char error = 0;
	SHT_TransmissionStart(); 					//transmission start
	error += SHT_WriteByte(STATUS_REG_W);	//send command to sensor
	error += SHT_WriteByte(value); 		//send value of status register
	return error; 						//error>=1 in case of no response form the sensor
}

void SHT_GetResults(float * shtRealTemp, int * shtRealHum)
{
	int humidity_sht;
	int temp_sht;
	unsigned int temp_ticks;
	unsigned int humidity_ticks;
	
	temp_ticks = SHT_Measure(SHT_TEMPERATURE); //to
	humidity_ticks = SHT_Measure(SHT_HUMIDITY); //to
		
	temp_sht = (int)temp_ticks;
	humidity_sht = (int)humidity_ticks;
	SHT_Calculate(&temp_sht, &humidity_sht);			//konwersja z binarki na odpowiedni¹ wartoœæ liczb rzeczywistych
	*shtRealTemp = (float)temp_sht/100;				//przeliczenie temperatury na wartoœæ z przecinkiem
	*shtRealHum = (int)humidity_sht/100;		//przeliczenie wilgotnoœci na wartoœæ z przecinkiem
}

unsigned char SHT_SoftReset(void)
//----------------------------------------------------------------------------------
// resets the sensor by a softreset
{
	unsigned char error = 0;
	SHT_TransmissionStart();						//reset communication
	error += SHT_WriteByte(RESET);			//send RESET-command to sensor
	return error;						//error=1 in case of no response form the sensor
}