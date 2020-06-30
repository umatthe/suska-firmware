#include <avr/io.h>
#include <stdint.h>

#include "pwm.h"

void pwm_init( uint16_t max)
{
	PWMDDR |= PWMPIN;

#if (_PWM_TIMER_ != 0)
        // Fast PWM-Mode 14
	// Takt Vorteiler 1	
	PWMTCCRxA = _BV(COM1C1) | _BV(WGM11);
	PWMTCCRxB = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
	PWMICRx = max;
#else
	// Fast PWM-Mode 3 (max fixed 0xff)
	// Takt Vorteiler 1
	PWMTCCRxA = _BV(COM0B1) | _BV(WGM00) | _BV(WGM01);
	PWMTCCRxB = _BV(CS00);
#endif
}

void pwm_set( uint16_t val)
{
#if (_PWM_TIMER_ != 0)
	if(val>PWMICRx) val=PWMICRx;
#else
	if(val>0xff) val=0xff;
#endif
	PWMOCRxy = val;
}

