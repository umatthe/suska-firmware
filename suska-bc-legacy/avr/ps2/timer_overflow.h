/*
 * Setup for timer 0 and 2
 */

#if defined(__AVR_ATmega644__)

#define t0_on_2ms()   { TCNT0 = 0; TCCR0B=0x04; }
#define t0_on_32ms()  { TCNT0 = 0; TCCR0B=0x05; }
#define t0_off()      { TCCR0B = 0; }
#define t0_overflow() ( TCCR0B==0 )

#define t2_on_2ms()   { TCNT2 = 0; TCCR2B=0x04; }
#define t2_on_32ms()  { TCNT2 = 0; TCCR2B=0x07; }
#define t2_off()      { TCCR2B = 0; }
#define t2_overflow() ( TCCR2B==0 )

#else

#define t0_on_2ms()   { TCNT0 = 0; TCCR0=0x04; }
#define t0_on_32ms()  { TCNT0 = 0; TCCR0=0x05; }
#define t0_off()      { TCCR0 = 0; }
#define t0_overflow() ( TCCR0==0 )

#define t2_on_2ms() 	{ TCNT2 = 0; TCCR2=0x04; }
#define t2_on_32ms() 	{ TCNT2 = 0; TCCR2=0x07; }
#define t2_off() 	 	  { TCCR2 = 0; }
#define t2_overflow() ( TCCR2==0 )

#endif

/* Prototypes */
void timer_init(void);

