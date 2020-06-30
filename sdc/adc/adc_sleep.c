#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "adc.h"

static volatile uint8_t done;
uint16_t adc_sample(void)
{
	done=0;
	set_sleep_mode(SLEEP_MODE_ADC);
	sleep_mode();                   // in den Schlafmodus wechseln
	
	while(!done); // Schleife, falls anderer Interrupt das Aufwachen veranlasste

	return adc_GetValue();
}

ISR(ADC_vect) 
{
	// ADC Conversion done --> Wake up
	done=1;
}
