/*
 * ADC.c
 *
 *  Created on: Sep 9, 2020
 *      Author: euler
 */


#include "adc.h"

void adc_init( void )
{
    /*------------ADC CONFIG------------*/

    // Tensión de referencia Vcc
    ADMUX &=~ (1<<REFS1);
    ADMUX |= (1<<REFS0);

    // Se configura el prescaler para que divida el clk por 128 (16 MHz / 128 = 125 Khz). El ACD debe funcionar entre 50 y 200 KHz según manual.
    ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

    // Se habilita el ADC
    ADCSRA |= (1<<ADEN);
}

uint16_t adc_read(uint8_t ch)
{
  // select the corresponding channel 0~7
  // ANDing with ’7′ will always keep the value
  // of ‘ch’ between 0 and 7
  ch &= 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0xF8)| ch; // clears the bottom 3 bits before ORing

  // start single convertion
  // write ’1′ to ADSC
  ADCSRA |= (1<<ADSC);

  // wait for conversion to complete
  // ADSC becomes ’0′ again
  // till then, run loop continuously
  while(ADCSRA & (1<<ADSC));

  return (ADC);
}
