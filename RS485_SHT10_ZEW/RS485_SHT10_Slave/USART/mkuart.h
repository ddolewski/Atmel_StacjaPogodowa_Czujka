/*
 * mkuart.h
 *
 *  Created on: 2010-09-04
 *       Autor: Miros³aw Kardaœ
 */

#ifndef MKUART_H_
#define MKUART_H_
#include <util/delay.h>
#include <stdio.h>
#define F_CPU			8000000
#define UART_BAUD		9600			// tu definiujemy interesuj¹c¹ nas prêdkoœæ
#define __UBRR F_CPU/16/UART_BAUD-1		// obliczamy UBRR dla U2X=0

#define UART_TX_BUF_SIZE		10

volatile char dataToSend[UART_TX_BUF_SIZE];

void USART_Init(uint16_t baud);

char uart_getc(void);
void uart_putc(char data);
void uart_puts(char *s);
void uart_putint(int value, int radix);

void USART_SendByte(char u8Data);
#endif /* MKUART_H_ */
