/////////////////////////////////////////////
//
// ADC Routines for AVR AD converter
//
// Stefan Weigl 18. Aug. 2007
//
////////////////////////////////////////////

#ifndef __ADC_H__
#define __ADC_H__

#include <avr/pgmspace.h>

#include "config.h"

#ifndef noinline
#define noinline __attribute__((noinline))
#endif

// Configurable Values

/*
    For full resolution the ADC clock needs to be configured
    in a ranga of 50kHz to 200kHz. So a prescaling dependent
    on F_CPU is required.
    Valid settings are:
                                        ADPSn
    clk[MHz]   prescale ADC clock     2  1  0
    -------------------------------------------
        1       8       125kHz        0  1  1
        1       16      62.5kHz       1  0  0
        8       64      125kHz        1  1  0
        8       128     62.5kHz       1  1  1
        12      128     93.75kHz      1  1  1
        12      64      187kHz        1  1  0
        16      128     125kHz        1  1  1
        20      128     156.25kHz     1  1  1
*/
#if   (F_CPU == 20000000)
#define ADCPRESCALE _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0)
#elif (F_CPU == 16000000)
#define ADCPRESCALE _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0)
#elif (F_CPU == 12000000)
#define ADCPRESCALE _BV(ADPS2) | _BV(ADPS1)
#elif (F_CPU ==  8000000)
#define ADCPRESCALE _BV(ADPS2) | _BV(ADPS1)
#elif (F_CPU ==  1000000)
#define ADCPRESCALE _BV(ADPS1) | _BV(ADPS0)
#else
#error "CPU-Freq not supported for ADC"
#endif


typedef uint8_t ADCREFVOLTAGE;

#define     ADCREF_AVcc 0
#define    ADCREF_INT256 1
#define    ADCREF_ARef 2

#define ADCINPUT_11 0x1e

#define adc_startConversion() ADCSRA |= _BV(ADSC)
#define adc_enableADC() ADCSRA |= _BV(ADEN)

void adc_setChannel(uint8_t ch);
//void setRefVoltage(ADCREFVOLTAGE ref);
void adc_init(ADCREFVOLTAGE ref);

uint8_t adc_ConvReady(void);
uint16_t adc_GetValue();

#endif //__ADC_H__
