//#include <avr/interrupt.h>
//#include <avr/io.h>
#include "timer_overflow.h"
/*
 * Initialize timer overflow interrupts
 */
void timer_init(void)
{
  TIMSK0 |= _BV(TOIE0); /* allow timer0 overflow */
  TIMSK2 |= _BV(TOIE2); /* allow timer2 overflow */
  t0_off();
  t2_off();
}

/*
 * Timer 2 overflow irpt
 */
ISR(TIMER2_OVF_vect)
{
  t2_off();
  TCNT2 = 0;
}

/*
 * Timer 0 overflow irpt
 */
ISR(TIMER0_OVF_vect)
{
  t0_off();
  TCNT0 = 0;
}
