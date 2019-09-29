/* Copyright 2015 Jack Humbert
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Simple analog to digitial conversion

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "analog.h"

static uint8_t aref = ADC_REF_POWER;  // default to AREF = Vcc

void analogReference(uint8_t mode) { aref = mode & 0xC0; }

uint8_t pinToADCIndex(uint8_t pin){
  #if defined(__AVR_ATmega32U4__)
    static const uint8_t PROGMEM pin_to_adc[] = {
      B4, 8,
      B5, 7,
      B6, 6,
      D4, 11,
      D6, 10,
      D7, 9,
      F0, 0,
      F1, 1,
      F4, 2,
      F5, 3,
      F6, 4,
      F7, 5
    };

    for ( uint8_t index = 0; index<sizeof(pin_to_adc)/2 ; ++index){
      uint8_t entry = pgm_read_byte(pin_to_adc + index*2);
      //uprintf("pinToADCIndex : entry %u : index : %u\n", entry, index);
      if (entry==pin){
        return pgm_read_byte(pin_to_adc + index*2 + 1);
      } else if (entry>pin){
        return (uint8_t)-1;
      }
    }
    return (uint8_t)-1;
  #elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__) || defined(__AVR_ATmega32A__)
    if (pin<A0 || pin>A7){
      return (uint8_t)-1;
    } else {
      return pin & ~(0xFF<<(PORT_SHIFTER));
    }
  #else
    #error "no implemented"
  #endif
}

// Arduino compatible pin input
int16_t analogRead(uint8_t pin) {
#if defined(__AVR_ATmega32U4__)
    static const uint8_t PROGMEM pin_to_mux[] = {0x00, 0x01, 0x04, 0x05, 0x06, 0x07, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20};
    if (pin >= 12) return 0;
    return adc_read(pgm_read_byte(pin_to_mux + pin));
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__) || defined(__AVR_ATmega32A__)
    if (pin >= 8) return 0;
    return adc_read(pin);
#else
    return 0;
#endif
}

// Mux input
int16_t adc_read(uint8_t mux) {
#if defined(__AVR_AT90USB162__)
    return 0;
#else
    uint16_t res = 0;

	ADCSRA = (1<<ADEN) | ADC_PRESCALER;		// enable ADC
  
#ifndef __AVR_ATmega32A__
	ADCSRB = (1<<ADHSM) | (mux & 0x20);		// high speed mode
#endif
	ADMUX = aref | (mux & 0x1F);			// configure mux input
	ADCSRA |= (1<<ADSC);	// start the conversion
	while (ADCSRA & (1<<ADSC)) ;			// wait for result
	res = ADCL;					// must read LSB first
  res |= (ADCH << 8) | res;
  
  ADCSRA &= ~(1<<ADEN);//turn off the ADC
  return res;
#endif
}
