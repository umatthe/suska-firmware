///////////////////////////////////////
//
// Interrupt Routine using Timer0 to 5 
// for defined delays
//
// U. Matthe, swe, H. Hoellerl
//
// Atmega 1280/1281/2560/2561
// Atmega 48/88/168/644
// Atmega 8 (only Timer0)
//
// Configuration Switches:
// #######################
// #define _USE_DELAY_
// 	Uses _delay_ms for delayms function (no timer/no interrupt)
// #define _TICK_HIRES_
// 	Uses Timer2 (1ms Tick) for delayms (Not supported for Atmega8)
// #define _SOFTPWM_
// #define _TICK_PWMDDR_
// #define _TICK_PWMPORT_
// #define _TICK_PWMPIN_
// 	one channel Softpwm at _TICK_PWMPIN_
// #define _USE_TIMER1_
// #define _USE_TIMER2_
// #define _USE_TIMER3_
// #define _USE_TIMER4_
// #define _USE_TIMER5_
//	Use Timer1-5 for tick. If no _USE_TIMERx_ is set, default TIMER0 is used.
//	Not supported for Atmega8
// #define SIGNALTICK
// 	adds interface for delay polling.
// #define DEBUGTICK
// #define _TICK_DEBUGDDR_
// #define _TICK_DEBUGPORT_
// #define _TICK_DEBUGPIN_
// 	enable ticksignal at _TICK_DEBUGPIN_
//
// Interfaces:
// ###########
// tick_init(tick_t speed)
// 	initializes timer and activates interrupt routine
// sleep(uint32_t ticks)
// 	delay for number of ticks
// sleepms(uint32_t ms)
// 	delay for ms milliseconds (resolution depends on timer speed)
// delayms(uint32_t ms)
// 	delay for ms milliseconds (resolution depends on timer speed/_TICK_HIRES_/_USE_DELAY_)
// 
// _SOFTPWM_:
// tick_initSoftPWM(uint8_t maxval)
//
// tick_setSoftPWM(uint8_t value);
// 
// SIGNALTICK:
// uint8_t tickReached();
//
// setTickSignal(uint32_t t);
//
//////////////////////////////////////

#ifndef __TICK_H__
#define __TICK_H__

#include <stdint.h>
#include "config.h"

#if !defined(__AVR_ATmega8__)
// Defines for "new" Atmegas
#include "avrtick.h"
#endif

#if defined(_USE_DELAY_) || defined (__AVR_ATmega649A__)
#include <util/delay.h>
#endif

#if defined (_TICK_HIRES_) & defined (_USE_DELAY_)
#error "_TICK_HIRES_ and _USE_DELAY_ not possible"
#endif
#if defined (_TICK_HIRES_) & defined (_USE_TIMER2_)
#error "_TICK_HIRES_ and _USE_TIMER2_ not possible"
#endif
#if defined(__AVR_ATmega8__) & (defined(_USE_TIMER1_) | defined(_USE_TIMER2_) | defined(_USE_TIMER3_) | defined(_USE_TIMER4_) | defined(_USE_TIMER5))
#error "Atmega8: Only TIMER0 possible"
#endif
extern volatile uint32_t tick;

#ifndef noinline
#define noinline __attribute__((noinline))
#endif

//////////////////////////////////////////////
//
// Map Atmega8 Names to Atmega644/168/88/48 
//
//////////////////////////////////////////////
#if defined(__AVR_ATmega8__)

#define _TIMERx_COMPA_vect_ TIMER0_OVF_vect
#define _OCIExA_ TOV0
#else
// New Atmega Architecture in avrtick.h
#endif

// ########################################
// ### ATMEGAxx section
// ########################################
#if !defined(__AVR_ATmega8__)
#if defined(_USE_TIMER1_)
#	define PRESCALE1MS   T1_PRESCALE1MS
#	define PRESCALE10MS  T1_PRESCALE10MS
#	define PRESCALE100MS T1_PRESCALE100MS
#	define PRESCALE1S    T1_PRESCALE1S
#elif defined(_USE_TIMER2_)
#	define PRESCALE1MS   T2_PRESCALE1MS
#	define PRESCALE10MS  T2_PRESCALE10MS
#	define PRESCALE100MS T2_PRESCALE100MS
#	define PRESCALE1S    T2_PRESCALE1S
#elif defined(_USE_TIMER3_)
#	define PRESCALE1MS   T3_PRESCALE1MS
#	define PRESCALE10MS  T3_PRESCALE10MS
#	define PRESCALE100MS T3_PRESCALE100MS
#	define PRESCALE1S    T3_PRESCALE1S
#elif defined(_USE_TIMER4_)
#	define PRESCALE1MS   T4_PRESCALE1MS
#	define PRESCALE10MS  T4_PRESCALE10MS
#	define PRESCALE100MS T4_PRESCALE100MS
#	define PRESCALE1S    T4_PRESCALE1S
#elif defined(_USE_TIMER5_)
#	define PRESCALE1MS   T5_PRESCALE1MS
#	define PRESCALE10MS  T5_PRESCALE10MS
#	define PRESCALE100MS T5_PRESCALE100MS
#	define PRESCALE1S    T5_PRESCALE1S
#else
#	define PRESCALE1MS   T0_PRESCALE1MS
#	define PRESCALE10MS  T0_PRESCALE10MS
#	define PRESCALE100MS T0_PRESCALE100MS
#	define PRESCALE1S    T0_PRESCALE1S
#endif
#else // defined(__AVR_ATmega8__)
// ########################################
// ### ATMEGA8 section
// ########################################
/////////////////////////////////////////////////
//
// Example calculation for timer0 (8 Bit) ATMEGA8
//
// Freq     = 20000000 Hz (20MHz)
// Tick     = 0.001s (1ms)
// Prescale = 256  
// Limit    = 256 - (Freq/Prescale*Tick) = 177.87 --> 178
/////////////////////////////////////////////////
#if   (F_CPU == 20000000) //######## 20MHz ####
	//Prescale==256
#	define PRESCALE1MS _BV(CS02)
#	define LIMIT1MS 178
	//Prescale==1024
#	define PRESCALE10MS _BV(CS02)|_BV(CS00)
#	define LIMIT10MS 61
#elif (F_CPU == 16000000) //######## 16MHz ####
	//Prescale==256
#	define PRESCALE1MS _BV(CS02)
#	define LIMIT1MS 193
	//Prescale==1024
#	define PRESCALE10MS _BV(CS02)|_BV(CS00)
#	define LIMIT10MS 100
#elif (F_CPU == 8000000) //######### 8MHz ####
	//Prescale=64
#	define PRESCALE1MS _BV(CS01)|_BV(CS00)
#       define LIMIT1MS 131
	//Prescale=1024
#	define PRESCALE10MS _BV(CS02)|_BV(CS00)
#       define LIMIT10MS 234
#elif (F_CPU == 1000000) //######### 1MHz ####
	//Prescale=8
#	define PRESCALE1MS _BV(CS01)
#       define LIMIT1MS 131
	//Prescale=64
#	define PRESCALE10MS _BV(CS01)|_BV(CS00)
#       define LIMIT10MS 100
#else
#	error "CPU-Freq not supported"
#endif
#endif



#ifdef DEBUGTICK
#if 	!defined(_TICK_DEBUGDDR_) | !defined(_TICK_DEBUGPORT_) | !defined(_TICK_DEBUGPIN_)
#		error "_TICK_DEBUGDDR_/_TICK_DEBUGPORT_/_TICK_DEBUGPIN_ undefined with DEBUGTICK enabled."
#	endif
#	define DEBUGDDR   _TICK_DEBUGDDR_
#	define DEBUGPORT _TICK_DEBUGPORT_
#	define DEBUGPIN  _TICK_DEBUGPIN_
#endif

#ifdef _SOFTPWM_
#ifdef _TICKPWMDDR_
#define TICKPWMDDR  _TICKPWMDDR_
#else
#warning "_TICKPWMDDR_ not defined use DDRB"
#define TICKPWMDDR  DDRB
#endif
#ifdef _TICKPWMPIN_
#define TICKPWMPIN  _TICKPWMPIN_
#else
#warning "_TICKPWMPIN_ not defined use PB7"
#define TICKPWMPIN  PB7
#endif
#ifdef _TICKPWMPORT_
#define TICKPWMPORT _TICKPWMPORT_
#else
#warning "_TICKPWMPORT_ not defined use PORTB"
#define TICKPWMPORT PORTB
#endif
#define tickPWMon()  TICKPWMPORT |= _BV(TICKPWMPIN)
#define tickPWMoff() TICKPWMPORT &=~_BV(TICKPWMPIN)
void tick_initSoftPWM(uint8_t maxval);
void tick_setSoftPWM(uint8_t value);
#endif


#ifdef SIGNALTICK
uint8_t tickReached();
void    setTickSignal(uint32_t t);
uint32_t getTicks( uint32_t msecs );
#endif

typedef enum {TICK1MS=1,TICK10MS=10,TICK100MS=100,TICK1S=1000} tick_t;

void tick_init( tick_t speed );
void noinline sleep( uint32_t );
void noinline sleepms( uint32_t );
void noinline delayms( uint32_t );

#endif //__TICK_H__
