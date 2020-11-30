/*
 * TIMER.cpp
 *
 *  Created on: Sep 9, 2020
 *      Author: euler
 */

#include "timer.h"

/*
Se utiliza el timer 5 del arduino mega2560
*/

#define F_INT (500) // frecuencia (Hz) del timer (muestreo)

void timer_init( void )
{
	 // Operation Mode = CTC
	 TCCR5A &=~ (1<<WGM50);
	 TCCR5A &=~ (1<<WGM51);
	 TCCR5B |=  (1<<WGM52);
	 TCCR5B &=~ (1<<WGM53);

	 // Compare value
	 OCR5A = (F_CPU/1024/F_INT) - 1;

	 // enable interrupt
	 TIMSK5 |= (1<<OCIE5A);
}

void timer_on( void ){

	 TCNT5 = 0x0000;
	 //N = 1024
	 TCCR5B |=  (1<<CS50);
	 TCCR5B &=~ (1<<CS51);
	 TCCR5B |=  (1<<CS52);

}

void timer_off( void ){

	 //Clock setting T1clock = 0 Hz
	 TCCR5B &=~ (1<<CS50);
	 TCCR5B &=~ (1<<CS51);
	 TCCR5B &=~ (1<<CS52);
}



/*
void timer_init( void )
{
	 //Operation Mode = CTC
	 TCCR1A &=~ (1<<WGM10);
	 TCCR1A &=~ (1<<WGM11);
	 TCCR1B |=  (1<<WGM12);
	 TCCR1B &=~ (1<<WGM13);

	 //Compare value
	 OCR1A = (F_CPU/1024/F_INT) - 1;

	 //enable interrupt
	 TIMSK1 |= (1<<OCIE1A);
}

void timer_on( void ){

	 TCNT1 = 0x0000;
	 //N = 1024
	 TCCR1B |=  (1<<CS10);
	 TCCR1B &=~ (1<<CS11);
	 TCCR1B |=  (1<<CS12);

}

void timer_off( void ){

	 //Clock setting T1clock = 0 Hz
	 TCCR1B &=~ (1<<CS10);
	 TCCR1B &=~ (1<<CS11);
	 TCCR1B &=~ (1<<CS12);
}
*/
