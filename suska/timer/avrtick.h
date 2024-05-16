///////////////////////////////////////
//
// Udo Matthe
//
// Atmega 1280/1281/2560/2561
// - Timer0/2      (8 Bit)
// - Timer1/3/4/5 (16 Bit)
// Atmega 48/88/168/644
// - Timer0/2  (8 Bit)
// - Timer1   (16 Bit)
//
//////////////////////////////////////

#ifndef __AVRTICK_H__
#define __AVRTICK_H__

#include <stdint.h>
#include "config.h"


/////////////////////////////////////////////////
//
// Example calculation for timer
//
// Freq     = 20000000 Hz
// Tick     = 0.001s
// Prescale = 256  
// Limit    = Freq/Prescale*Tick-1 = 77.125 -->77
//
/////////////////////////////////////////////////


#if ((F_CPU == 8000000) | (F_CPU == 10000000) | (F_CPU == 14745600) | (F_CPU == 16000000) | (F_CPU == 20000000) |  (F_CPU == 32000000) | (F_CPU == 48000000) | (F_CPU == 56000000) | (F_CPU == 60000000) | (F_CPU == 64000000) )
	//Prescale==256
#	define T0_PRESCALE1MS _BV(CS02)
#	define T1_PRESCALE1MS _BV(CS02)
#	define T2_PRESCALE1MS _BV(CS02)|_BV(CS01)
#	define T3_PRESCALE1MS _BV(CS02)
#	define T4_PRESCALE1MS _BV(CS02)
#	define T5_PRESCALE1MS _BV(CS02)
	//Prescale==1024
#	define T0_PRESCALE10MS _BV(CS02)|_BV(CS00)
#	define T1_PRESCALE10MS _BV(CS02)|_BV(CS00)
#	define T2_PRESCALE10MS _BV(CS02)|_BV(CS01)|_BV(CS00)
#	define T3_PRESCALE10MS _BV(CS02)|_BV(CS00)
#	define T4_PRESCALE10MS _BV(CS02)|_BV(CS00)
#	define T5_PRESCALE10MS _BV(CS02)|_BV(CS00)
	//Prescale==1024
#	define T1_PRESCALE100MS _BV(CS02)|_BV(CS00)
#	define T3_PRESCALE100MS _BV(CS02)|_BV(CS00)
#	define T4_PRESCALE100MS _BV(CS02)|_BV(CS00)
#	define T5_PRESCALE100MS _BV(CS02)|_BV(CS00)
	//Prescale==1024
#	define T1_PRESCALE1S _BV(CS02)|_BV(CS00)
#	define T3_PRESCALE1S _BV(CS02)|_BV(CS00)
#	define T4_PRESCALE1S _BV(CS02)|_BV(CS00)
#	define T5_PRESCALE1S _BV(CS02)|_BV(CS00)
#elif (F_CPU == 1000000)
	//Prescale=8
#	define T0_PRESCALE1MS _BV(CS01)
#	define T1_PRESCALE1MS _BV(CS01)
#	define T2_PRESCALE1MS _BV(CS01)
#	define T3_PRESCALE1MS _BV(CS01)
#	define T4_PRESCALE1MS _BV(CS01)
#	define T5_PRESCALE1MS _BV(CS01)
	//Prescale=64
#	define T0_PRESCALE10MS _BV(CS01)|_BV(CS00)
#	define T1_PRESCALE10MS _BV(CS01)|_BV(CS00)
#	define T2_PRESCALE10MS _BV(CS02)
#	define T3_PRESCALE10MS _BV(CS01)|_BV(CS00)
#	define T4_PRESCALE10MS _BV(CS01)|_BV(CS00)
#	define T5_PRESCALE10MS _BV(CS01)|_BV(CS00)
	//Prescale=64
#	define T1_PRESCALE100MS _BV(CS01)|_BV(CS00)
#	define T3_PRESCALE100MS _BV(CS01)|_BV(CS00)
#	define T4_PRESCALE100MS _BV(CS01)|_BV(CS00)
#	define T5_PRESCALE100MS _BV(CS01)|_BV(CS00)
	//Prescale=64
#	define T1_PRESCALE1S _BV(CS01)|_BV(CS00)
#	define T3_PRESCALE1S _BV(CS01)|_BV(CS00)
#	define T4_PRESCALE1S _BV(CS01)|_BV(CS00)
#	define T5_PRESCALE1S _BV(CS01)|_BV(CS00)
#else
#       error "CPU-Frequenz not supported"
#endif

// ################################################
// ### Timer Limits for 1ms/10ms/100ms/1s
// ### 100ms and 1s only possible for 16 Bit Timer
// ################################################
#if !defined(__AVR_ATmega8__)
#if   (F_CPU == 64000000)  // Prescale 256/1024
#	define LIMIT1MS     246
#	define LIMIT10MS    621
#	define LIMIT100MS  6246
#	define LIMIT1S    62496
#elif   (F_CPU == 60000000)  // Prescale 256/1024
#	define LIMIT1MS     231
#	define LIMIT10MS    582
#	define LIMIT100MS  5856
#	define LIMIT1S    58590
#elif   (F_CPU == 56000000)  // Prescale 256/1024
#	define LIMIT1MS     216
#	define LIMIT10MS    544
#	define LIMIT100MS  5466
#	define LIMIT1S    54684
#elif   (F_CPU == 48000000)  // Prescale 256/1024
#	define LIMIT1MS     185 
#	define LIMIT10MS    466
#	define LIMIT100MS  4685
#	define LIMIT1S    46872
#elif (F_CPU == 32000000)  // Prescale 256/1024
#	define LIMIT1MS     122
#	define LIMIT10MS    310
#	define LIMIT100MS  3122
#	define LIMIT1S    31248
#elif (F_CPU == 20000000)  // Prescale 256/1024
#	define LIMIT1MS      77
#	define LIMIT10MS    194
#	define LIMIT100MS  1952
#	define LIMIT1S    19530
#elif (F_CPU == 16000000)  // Prescale 256/1024
#	define LIMIT1MS      61
#	define LIMIT10MS    155
#	define LIMIT100MS  1561
#	define LIMIT1S    15624
#elif (F_CPU == 14745600)  // Prescale 256/1024
#       define LIMIT1MS     56
#       define LIMIT10MS   143
#	define LIMIT100MS 1439
#	define LIMIT1S   14399 
#elif (F_CPU == 10000000)  // Prescale 256/1024
#	define LIMIT1MS     39
#	define LIMIT10MS    97
#	define LIMIT100MS  975
#	define LIMIT1S    9764 
#elif (F_CPU == 8000000)  // Prescale 256/1024
#	define LIMIT1MS     30 
#	define LIMIT10MS    77
#	define LIMIT100MS  782
#	define LIMIT1S    7811 
#elif (F_CPU == 1000000)  // Prescale 8/64
#	define LIMIT1MS     124
#	define LIMIT10MS    155
#	define LIMIT100MS  1561
#	define LIMIT1S    15624
#else
#	error "CPU-Freq not supported"
#endif
#endif

#if defined(_USE_TIMER1_)
//   Use Timer1 (16 Bit)
#    define TIMERx_COMPA_vect TIMER1_COMPA_vect
#    define OCIExA OCIE1A
#    define TCCRxA TCCR1A
#    define TCCRxB TCCR1B
#    define OCRxA  OCR1A
#    define TIMSKx TIMSK1
//   Select Mode CTC
#    define TCCRAval 0
#    define TCCRBval _BV(WGM12)
#elif defined(_USE_TIMER2_)
//   Use Timer2 (8 Bit)
#    define TIMERx_COMPA_vect TIMER2_COMPA_vect
#    define OCIExA OCIE2A
#    define TCCRxA TCCR2A
#    define TCCRxB TCCR2B
#    define OCRxA  OCR2A
#    define TIMSKx TIMSK2
//   Select Mode CTC
#    define TCCRAval _BV(WGM21)
#    define TCCRBval 0
#elif defined(_USE_TIMER3_)
//   Use Timer3 (16 Bit)
#    define TIMERx_COMPA_vect TIMER3_COMPA_vect
#    define OCIExA OCIE3A
#    define TCCRxA TCCR3A
#    define TCCRxB TCCR3B
#    define OCRxA  OCR3A
#    define TIMSKx TIMSK3
//   Select Mode CTC
#    define TCCRAval 0
#    define TCCRBval _BV(WGM32)
#elif defined(_USE_TIMER4_)
//   Use Timer4 (16 Bit)
#    define TIMERx_COMPA_vect TIMER4_COMPA_vect
#    define OCIExA OCIE4A
#    define TCCRxA TCCR4A
#    define TCCRxB TCCR4B
#    define OCRxA  OCR4A
#    define TIMSKx TIMSK4
//   Select Mode CTC
#    define TCCRAval 0
#    define TCCRBval _BV(WGM42)
#elif defined(_USE_TIMER5_)
//   Use Timer5 (16 Bit)
#    define TIMERx_COMPA_vect TIMER5_COMPA_vect
#    define OCIExA OCIE5A
#    define TCCRxA TCCR5A
#    define TCCRxB TCCR5B
#    define OCRxA  OCR5A
#    define TIMSKx TIMSK5
//   Select Mode CTC
#    define TCCRAval 0
#    define TCCRBval _BV(WGM52)
#else
//   Use Timer0 (8 Bit)
#    define OCRxA  OCR0A
#    define TIMSKx TIMSK0
#    define OCIExA OCIE0A
#    define TCCRxA TCCR0A
#if !defined(__AVR_ATmega649A__)
#    define TIMERx_COMPA_vect TIMER0_COMPA_vect
#    define TCCRxB TCCR0B
//   Select Mode CTC
#    define TCCRAval _BV(WGM01)
#    define TCCRBval 0
#else // ATMEGA 649 (suska-bf)
#    define TIMERx_COMPA_vect TIMER0_COMP_vect
//   Select Mode CTC
#    define TCCRAval _BV(WGM01)
#endif
#endif

#endif //__AVRTICK_H__
