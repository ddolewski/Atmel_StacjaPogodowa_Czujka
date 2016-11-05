/*
 * mkuart.c
 *
 *  Created on: 2010-09-04
 *       Autor: Miroslaw Kardas
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "mkuart.h"

volatile uint8_t byteIndexToSent = 0;
char *test="1234567890\r\n";
volatile uint8_t msgLength = 10;

void USART_Init(uint16_t baud) 
{
	/* Ustawienie predkosci */
	UBRRH = (uint8_t)(baud >> 8);
	UBRRL = (uint8_t)baud;
	/* Zalczenie nadajnika I odbiornika */
	UCSRB |= (1 << TXEN);// | (1 << UDRIE0);
}

void uart_puts(char *s)		// wysy³a ³añcuch z pamiêci RAM na UART
{
	register char c;
	while ((c = *s++)) 
	uart_putc(c);			// dopóki nie napotkasz 0 wysy³aj znak
}

void uart_putint(int value, int radix)	// wysy³a na port szeregowy tekst
{
	char string[2];			// bufor na wynik funkcji itoa
	sprintf(string, "%i\r\n", value);	// konwersja value na ASCII
	uart_puts(string);			// wyœlij string na port szeregowy
}

void USART_SendByte(char u8Data)
{
	while((UCSRA &(1<<UDRE)) == 0);
	UDR = u8Data;
}

ISR(USART_UDRE_vect)
{
	if (byteIndexToSent < msgLength)
	{
		UDR = dataToSend[byteIndexToSent];
		byteIndexToSent++;
	}
	else
	{
		byteIndexToSent = 0;
		UCSRB &= ~(1<<UDRIE);
	}
}