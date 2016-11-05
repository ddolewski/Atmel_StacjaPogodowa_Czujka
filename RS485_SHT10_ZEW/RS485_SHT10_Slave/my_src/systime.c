/*
 * systime.c
 *
 * Created: 2016-02-23 22:29:20
 *  Author: Doles
 */ 

#include "systime.h"
//#include "global.h"

static volatile systime_t sysTimer = 0;	//timer systemowy odmierzajacy czas w ms

//-------------------------------------------------------------------------------------------------
// Funkcja obslugi przerwania compare
//void
//return:		void
//-------------------------------------------------------------------------------------------------
ISR(TIMER2_COMP_vect)
{
	//PORTC ^= (1 << 7); //testowo
	sysTimer++;
}
//-------------------------------------------------------------------------------------------------
// Funkcja inicjalizuj¹ca timer systemeowy
//void
//return:		void
//-------------------------------------------------------------------------------------------------
void systimeInit(void)
{
	// ustawienie TIMER0 na 1kHz
	TCCR2 = ((1<<WGM21) | (1<<CS22));				// tryb CTC
	OCR2 = 124;						// dla 12MHz z AVR Timer Kalkulator (atnel)
	TIMSK = (1<<OCIE2);				// zezwolenie na przerwanie CompareMatch
}
//-------------------------------------------------------------------------------------------------
// Funkcja zwracajaca aktualny czas systemowy
//void
//return:	sysTimer	czas systemowy
//-------------------------------------------------------------------------------------------------
systime_t systimeGet (void)
{
	return sysTimer;
}
//-------------------------------------------------------------------------------------------------
// Funkcja zwracajaca aktualny czas systemowy
//xLastTime		/inOut: wskaznik na zmienna reprezentujac¹ czas ostatniego wywolania danego timeouta
//xCheckTime	/in:	interwal czasu sprawdzenia
//return:	timeElapsed	TRUE  -  czas minal, FALSE - czas nie minal
//-------------------------------------------------------------------------------------------------
uint8_t systimeTimeoutControl (systime_t * xLastTime, systime_t xCheckTime)
{
	uint8_t  timeElapsed 	= FALSE;
	systime_t currentTime	= systimeGet();		//pobranie aktualnego czasu

	if (currentTime - (*xLastTime) >= xCheckTime)
	{
		*xLastTime = currentTime;
		timeElapsed = TRUE;
	}
	return timeElapsed;
}
//-----------------------------------------------------------------------------
// Funkcja ztrzymujaca wykonywanie programu na podany czas
// xTimeMs/in czas zatrzymania
// return: void
//-----------------------------------------------------------------------------
void systimeDelayMs(uint32_t xTimeMs)
{
	systime_t time = systimeGet();
	while (!systimeTimeoutControl(&time, xTimeMs))
	{
		asm("nop");
	}
}