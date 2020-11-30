/*
 * ADC.h
 *
 *  Created on: Sep 9, 2020
 *      Author: euler
 */

#ifndef ADC_H_
#define ADC_H_


#include <Arduino.h>

void adc_init( void );

uint16_t adc_read( uint8_t );


#endif /* ADC_H_ */
