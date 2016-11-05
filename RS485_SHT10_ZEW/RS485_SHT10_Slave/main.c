/*
 * main.c
 *
 * Created: 2016-01-16 19:33:23
 * Author : Damian Dolewski
 * PROGRAM DO MINI STACJI POGODOWEJ -> CZUJNIK NA ZEWN¥TRZ
 * POMIARY: WILGOTNOŒÆ + TEMPERATURA (SHT10)
 * KOMUNIKACJA: RS485
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "string.h"
#include "USART/mkuart.h"
#include "SHT10/Sht.h"
#include "my_inc/systime.h"

//#define SHT_STATUS_DEBUG

#define START_BYTE_S	83	//'S'
#define START_BYTE_T	84	//'T'
#define START_BYTE_A	65	//'A'

#define END_BYTE_CR		13	//'\n'
#define END_BYTE_LF		10	//'\r'

#define CHAR_S			83
#define CHAR_H			72
#define CHAR_T			84
#define CHAR_E			69
#define CHAR_R			82
	
#define ALWAYS			1

typedef union
{
	unsigned char tempBytes[4];
	float tempFloat;
}real_temp;

typedef struct sht_measurement_t
{
	real_temp shtTemp;
	int shtHum;	
}sht_measurement_t;

static systime_t shtResetTimer = 0;
static systime_t shtGetResultsTimer = 0;
static systime_t rs485Timer = 0;

static void SendDataToMaster(sht_measurement_t * shtData);
static void SHT_FirstMeasurementProccess(sht_measurement_t * shtData);

int main(void)
{
	_delay_ms(50);
	systimeInit();
	sei();
	
	USART_Init(__UBRR);
	SHT_Init();

#ifdef SHT_STATUS_DEBUG
	char statusreg = 0;
	unsigned char shterr = 0;
	statusreg = SHT_ReadStatusReg(&shterr);
#endif

	static sht_measurement_t shtResults;
	SHT_FirstMeasurementProccess(&shtResults);
	SendDataToMaster(&shtResults);

    while (ALWAYS) 
    {
		if (systimeTimeoutControl(&shtResetTimer, 500))
		{
			SHT_Reset();
		}
		
		if (systimeTimeoutControl(&shtGetResultsTimer, 1000))
		{
			SHT_GetResults(&shtResults.shtTemp.tempFloat, &shtResults.shtHum);
		}
		
		if (systimeTimeoutControl(&rs485Timer, 1000))
		{
			
#ifndef SHT_STATUS_DEBUG
			SendDataToMaster(&shtResults);
#endif
			
#ifdef SHT_STATUS_DEBUG	
			USART_SendByte(statusreg);
#endif
		}
    }
	
	return 0;
}

static void SendDataToMaster(sht_measurement_t * shtData)
{
	memset(dataToSend, NULL, sizeof(dataToSend));
	
	dataToSend[0] = START_BYTE_S;
	dataToSend[1] = START_BYTE_T;
	dataToSend[2] = START_BYTE_A;
	dataToSend[8] = END_BYTE_CR;
	dataToSend[9] = END_BYTE_LF;
	
	if (shtData->shtHum == 0 || shtData->shtTemp.tempFloat == (float)(-40))
	{
		dataToSend[3] = CHAR_S;
		dataToSend[4] = CHAR_H;
		dataToSend[5] = CHAR_T;
		dataToSend[6] = CHAR_E;
		dataToSend[7] = CHAR_R;
	}
	else
	{
		dataToSend[3] = (char)shtData->shtHum;
		dataToSend[4] = shtData->shtTemp.tempBytes[0];
		dataToSend[5] = shtData->shtTemp.tempBytes[1];
		dataToSend[6] = shtData->shtTemp.tempBytes[2];
		dataToSend[7] = shtData->shtTemp.tempBytes[3];		
	}

	UCSRB |= (1 << UDRIE);
}

static void SHT_FirstMeasurementProccess(sht_measurement_t * shtData)
{
	SHT_Reset();
	systimeDelayMs(1000);
	SHT_GetResults(&shtData->shtTemp.tempFloat, &shtData->shtHum);
}