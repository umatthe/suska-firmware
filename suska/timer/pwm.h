#ifndef __NWM_H__
#define __NWM_H__

#include <stdint.h>
#include "config.h"

// Interface
void pwm_init( uint16_t max );
void pwm_set( uint16_t val);


// Configuration Check
#if !defined(_PWM_TIMER_) | !defined(_PWM_CHANNEL_)
#warning "_PWM_TIMER_ and/or _PWM_CHANNEL_ not defined using Timer1==1 ChannelC=3"
#define _PWM_TIMER_ 1
#define _PWM_CHANNEL_ 3
#endif

#if defined(__AVR_ATmega1280__) | defined(__AVR_ATmega1281__) | defined(__AVR_ATmega2560__) | defined(__AVR_ATmega2561__)

#if (_PWM_TIMER_ == 0)
#	define PWMTCCRxA TCCR0A
#	define PWMTCCRxB TCCR0B
#	if (_PWM_CHANNEL_ == 1)
#		define PWMOCRxy  OCR0A
#		define PWMDDR    DDRB
#		define PWMPIN    PB7
#	elif (_PWM_CHANNEL_ == 2)
#		define PWMOCRxy  OCR0B
#		define PWMDDR    DDRG
#		define PWMPIN    PG5
#       else
#	error "_PWM_CHANNEL_ not valid"
#	endif
#elif (_PWM_TIMER_ == 1)
#	define PWMTCCRxA TCCR1A
#	define PWMTCCRxB TCCR1B
#	define PWMICRx   ICR1
#	if (_PWM_CHANNEL_ == 1)
#		define PWMOCRxy  OCR1A
#		define PWMDDR    DDRB
#		define PWMPIN    PB5
#	elif (_PWM_CHANNEL_ == 2)
#		define PWMOCRxy  OCR1B
#		define PWMDDR    DDRB
#		define PWMPIN    PB6
#	elif (_PWM_CHANNEL_ == 3)
#		define PWMOCRxy  OCR1C
#		define PWMDDR    DDRB
#		define PWMPIN    PB7
#       else
#	error "_PWM_CHANNEL_ not valid"
#	endif
#elif (_PWM_TIMER_ == 3)
#	define PWMTCCRxA TCCR3A
#	define PWMTCCRxB TCCR3B
#	define PWMICRx   ICR3
#	if (_PWM_CHANNEL_ == 1)
#		define PWMOCRxy  OCR3A
#		define PWMDDR    DDRE
#		define PWMPIN    PE3
#	elif (_PWM_CHANNEL_ == 2)
#		define PWMOCRxy  OCR3B
#		define PWMDDR    DDRE
#		define PWMPIN    PE4
#	elif (_PWM_CHANNEL_ == 3)
#		define PWMOCRxy  OCR3C
#		define PWMDDR    DDRE
#		define PWMPIN    PE5
#       else
#	error "_PWM_CHANNEL_ not valid"
#	endif
#else
#	error "_PWM_TIMER_ not valid"
#endif
#elif defined(__AVR_ATmega644__)
#if (_PWM_TIMER_ == 1)
#	define PWMTCCRxA TCCR1A
#	define PWMTCCRxB TCCR1B
#	define PWMICRx   ICR1
#	if (_PWM_CHANNEL_ == 1)
#		define PWMOCRxy  OCR1A
#		define PWMDDR    DDRD
#		define PWMPIN    PD5
#	elif (_PWM_CHANNEL_ == 2)
#		define PWMOCRxy  OCR1B
#		define PWMDDR    DDRD
#		define PWMPIN    PD4
#       else
#	error "_PWM_CHANNEL_ not valid"
#	endif
#else
#	error "_PWM_TIMER_ not valid"
#endif
#else
#error "CPU not supported"
#endif



#endif
//
