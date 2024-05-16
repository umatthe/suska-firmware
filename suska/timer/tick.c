///////////////////////////////////
//
// Interrupt Routine using Timer0-5
// for defined delays
//
// U. Matthe, swe, H. Hoellerl
//
// Atmega 1280/1281/2560/2561
// Atmega 48/88/168/644
// Atmega 8 (only Timer0)
//
// Config/Usage Information in tick.h
//
///////////////////////////////////

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "tick.h"

volatile uint32_t tick=0;
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega16__)
static uint8_t counterPreset;
#endif
static uint32_t resolution;

#ifdef SIGNALTICK
volatile uint32_t tickSig = 0;
tick_t tickSpeed = (tick_t)0;
#endif

#ifdef _SOFTPWM_
static volatile uint8_t tickPWM=0;
static uint8_t tickPWMmax;
static uint8_t tickPWMval;
#endif

#ifdef _TICK_HIRES_
static volatile uint32_t mstick=0;
#endif

//
// Init Ticktimer
//
// Parameter: speed=TICK1MS|TICK10MS|(TICK100MS|TICK1S)
//
void tick_init( tick_t speed )
{
#if !defined _USE_DELAY_
#ifdef DEBUGTICK
  DEBUGDDR|=_BV(DEBUGPIN);
#endif
#ifdef SIGNALTICK
  tickSpeed=speed;
#endif

  cli();
// ########################################
// ### ATMEGAxx section
// ########################################
#if !defined(__AVR_ATmega8__) && !defined(__AVR_ATmega16__)
    TCCRxA = TCCRAval;
#if !defined(__AVR_ATmega649A__)
    TCCRxB = TCCRBval;
#endif
    if(speed==TICK1MS)
    {
        // Init Timer0 for 1ms Ticks:
#if !defined(__AVR_ATmega649A__) 
        TCCRxB |= PRESCALE1MS;
#else
        TCCRxA |= PRESCALE1MS;
#endif
        OCRxA = LIMIT1MS;
	resolution=1;
    }
#if defined(_USE_TIMER1_) | defined(_USE_TIMER3_) | defined(_USE_TIMER4_) | defined(_USE_TIMER5_)
    else if(speed==TICK100MS)
    {
        // Init Timer0 for 100ms Ticks:
        TCCRxB |= PRESCALE100MS;
        OCRxA = LIMIT100MS;
	resolution=100;
    }
    else if(speed==TICK1S)
    {
        // Init Timer0 for 1s Ticks:
        TCCRxB |= PRESCALE1S;
        OCRxA = LIMIT1S;
	resolution=1000;
    }
#endif
    else
    {
        // Init Timer0 for 10ms Ticks:
#if !defined(__AVR_ATmega649A__) 
        TCCRxB |= PRESCALE10MS;
#else
        TCCRxA |= PRESCALE10MS;
#endif
        OCRxA = LIMIT10MS;
	resolution=10;
    }

    TIMSKx |= _BV(OCIExA);

#ifdef _TICK_HIRES_
    // USE Timer2 for Hires Timer
    TCCR2A = _BV(WGM21);
    TCCR2B = T2_PRESCALE1MS;
    OCR2A = LIMIT1MS;
    TIMSK2 |= _BV(OCIE2A);
#endif
#else
// ########################################
// ### ATMEGA8 section
// ########################################
    if(speed==TICK1MS)
    {
        // Init Timer0 for 1ms Ticks:
        TCCR0 = PRESCALE1MS;
        // Atmega8 timer0 has no limit (Limit=255),
        // but the counter register itself can be preset
        TCNT0 = LIMIT1MS;
        counterPreset = LIMIT1MS;
	resolution=1;
    }
    else
    {
        // Init Timer0 for 10ms Ticks:
        TCCR0 = PRESCALE10MS;
        TCNT0 = LIMIT10MS;
        counterPreset = LIMIT10MS;
	resolution=10;
    }
    TIMSK = _BV(TOV0);
#endif
    sei();
#endif
}

//
// function sleep:
//
// Waits x ms or 10ms (see tick_init parameter speed)
//
#if 0
void noinline sleep(uint32_t x)
{
	// Note: Problem when tick wraps around
	x=x+tick;
	while(tick<x);
}
#endif
void noinline delayms(uint32_t ms)
{
#if defined(_TICK_HIRES_)
	// Note: Problem when tick wraps around
	ms=ms+mstick;
	while(mstick<ms);
#elif defined(_USE_DELAY_) || defined(__AVR_ATmega649A__)
	for(uint16_t i=0;i<ms;i++)
	{
		_delay_ms(1);
	}
#else
	sleepms(ms);
#endif
}

void noinline sleepms(uint32_t ms)
{
#if defined(_USE_DELAY_) || defined(__AVR_ATmega649A__)
	delayms(ms);
#else
//	uint32_t ticks=ms/resolution;
//	sleep(ticks);
	uint32_t ticks=ms/resolution+tick;
        while(tick<ticks);
#endif
}




#if !defined _USE_DELAY_
//
// Tick Interrupt Routine
//

ISR(TIMERx_COMPA_vect)
{
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega16__)
    TCNT0 = counterPreset;
#endif
 tick++;
#ifdef DEBUGTICK
 DEBUGPORT ^= _BV(DEBUGPIN);
#endif 
#ifdef SIGNALTICK
 if (tickSig != 0)
    tickSig--;
#endif
#ifdef _SOFTPWM_
 if(tickPWMmax!=tickPWMval)
 {
	 tickPWM--;
	 if(tickPWM==0)
	 {
		 tickPWM=tickPWMmax;
		 tickPWMon();
	 }
	 if(tickPWM==tickPWMval)
	 {
		 tickPWMoff();
	 }
 }
#endif
}

// Used to poll if some time has been passed
// no busy wait like sleep

#ifdef SIGNALTICK
uint8_t tickReached()
{
    if (tickSig == 0)
        return 1;
    else
        return 0;
}

// in increments configured  (1ms/10ms)
void setTickSignal(uint32_t t)
{
    tickSig = t;
}

uint32_t getTicks( uint32_t msecs )
{
	if(tickSpeed!=0)
	return (msecs/tickSpeed);
	else
	return 0;
}

#endif

#ifdef _SOFTPWM_
void tick_initSoftPWM(uint8_t maxval)
{
	TICKPWMDDR|=TICKPWMPIN;
	tickPWMmax=maxval;
	tickPWMval=maxval;
	tickPWM=maxval;

}

void tick_setSoftPWM(uint8_t value)
{
	if(value>tickPWMmax)
	{
		tickPWMval=tickPWMmax;
	}
	else
	{
		tickPWMon();
		tickPWMval=value;
	}

}
#endif


#ifdef _TICK_HIRES_
//
// 1ms Tick Interrupt Routine
//

ISR(TIMER2_COMPA_vect)
{
     mstick++;
}
#endif
#endif
