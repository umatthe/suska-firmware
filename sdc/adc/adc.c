/////////////////////////////////////////////
//
// ADC Routines for AVR AD converter
//
// Stefan Weigl 18. Aug. 2007
//
////////////////////////////////////////////

#include "adc.h"
#ifdef _ADC_SLEEP_
#include "adc_sleep.h"
#endif

/* 
    Reference selection
    REFS1  REFS0
    0       0       ARef, internal turned off
    0       1       AVcc with ext. cap at ARef
    1       0       reserved
    1       1       int 2.56V ref with ext. cap at AVcc
*/
noinline void adc_init(ADCREFVOLTAGE ref)
{
#ifdef _ADC_SLEEP_
	adc_sleep_init();
#endif
	switch(ref)
	{
		case ADCREF_AVcc:
			ADMUX = (ADMUX & ~(_BV(REFS0) | _BV(REFS1))) | _BV(REFS0);
			break;

		case ADCREF_INT256:
			ADMUX |= _BV(REFS0) | _BV(REFS1);
			break;

		case ADCREF_ARef:
			ADMUX = ADMUX & ~(_BV(REFS0) | _BV(REFS1));
			break;

		default:
			ADMUX |= _BV(REFS0) | _BV(REFS1); // use internal
			break;
	}

	// set ADC prescale
	ADCSRA = (ADCSRA & ~(_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0))) | (ADCPRESCALE);

	// set channel to 0 (default)
	ADMUX &= 0xE0;
	// single conversion is default

	adc_enableADC();
}

noinline void adc_setChannel(uint8_t ch)
{
    ADMUX = (ADMUX & 0xE0) | ch;
}

noinline uint8_t adc_ConvReady(void)
{
    if ((ADCSRA &  _BV(ADSC)) == 0)
    {
        return (1);
    } else
    {
        return (0);
    }
}

uint16_t adc_GetValue()
{
    //return(ADCH << 8 | ADCL);
    return (ADCW);
}
