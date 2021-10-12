#include <avr/interrupt.h>
#include <avr/io.h>
#include "timer_overflow.h"

//#define TEST

/*
 * Initialize timer overflow interrupts
 */
void timer_init(void)
{
#if defined(__AVR_ATmega644__)
  TIMSK0 |= _BV(TOIE0); /* allow timer0 overflow */
  TIMSK2 |= _BV(TOIE2); /* allow timer2 overflow */
#else
  TIMSK |= _BV(TOIE0); /* allow timer0 overflow */
	TIMSK |= _BV(TOIE2); /* allow timer2 overflow */
#endif
  t0_off();
  t2_off();
}


/*
 * Timer 2 overflow irpt
 */
ISR(TIMER2_OVF_vect)
{
#if defined(__AVR_ATmega644__)
  TCCR2B = 0; /* stop timer2 and reset TCCR2B to indicate the overflow */
#else
  TCCR2 = 0; /* stop timer2 and reset TCCR2 to indicate the overflow */
#endif
  TCNT2 = 0;
}

/*
 * Timer 0 overflow irpt
 */
ISR(TIMER0_OVF_vect)
{
#if defined(__AVR_ATmega644__)
  TCCR0B = 0; /* stop timer2 and reset TCCR0B to indicate the overflow */
#else
  TCCR0 = 0; /* stop timer0 and reset TCCR0 to indicate the overflow */
#endif
  TCNT0 = 0;
}

#ifdef TEST
int main()
{
	timer_init();
	sei(); /* enable irpts */

	/* use PD2 as output to enable meassurement of a signal */
	DDRD |= _BV(PD2);
	for(;;) {
		PORTD |= _BV(PD2);
		t0_on_32ms(); /* 32 ms high on PD2, 1 ms low */
		while(!t0_overflow());
		PORTD &= ~_BV(PD2);
		_delay_ms(1);
	}
}
#endif
