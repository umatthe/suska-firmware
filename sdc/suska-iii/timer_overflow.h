/*
 * Setup for timer 0 and 2
 */
#include <avr/interrupt.h>
#include <avr/io.h>


#define t0_on_2ms()   { TCNT0 = 0; TCCR0A=0x04; }
#define t0_on_32ms()  { TCNT0 = 0; TCCR0A=0x05; }
#define t0_off()      { TCCR0A = 0; }
#define t0_overflow() ( TCCR0A==0 )
//#define t0_overflow() ( 1==0 )

#define t2_on_2ms()   { TCNT2 = 0; TCCR2A=0x04; }
#define t2_on_32ms()  { TCNT2 = 0; TCCR2A=0x07; }
#define t2_off()      { TCCR2A = 0; }
#define t2_overflow() ( TCCR2A==0 )

/* Prototypes */
void timer_init(void);

