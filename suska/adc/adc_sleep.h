#ifndef _ADC_SLEEP_H_
#define _ADC_SLEEP_H_

uint16_t adc_sample(void);

#define adc_sleep_init() ADCSRA|=_BV(ADIE)

#endif
