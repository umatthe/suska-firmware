#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "pindefs.h"

void bias_step(void)
{
        _delay_ms(100);
        OUT_VBIAS_UD=1;
        _delay_ms(100);
        OUT_VBIAS_UD=0;
        _delay_ms(100);
}

void bias_plus(void)
{
        OUT_VBIAS_UD=1;
        _delay_ms(100);
        OUT_VBIAS_CS=0;
        _delay_ms(100);
        OUT_VBIAS_UD=0;

        bias_step();

        OUT_VBIAS_CS=1;
}

void bias_minus(void)
{
        OUT_VBIAS_UD=0;
        _delay_ms(100);
        OUT_VBIAS_CS=0;
        
        bias_step();

        OUT_VBIAS_CS=1;
}

void bias_init(void)
{
        OUT_VBIAS_UD = OUT_VBIAS_CS = 1;
        OUT_VBIAS_UD_DDR = OUT_VBIAS_CS_DDR = 1; /* output */
}

void bias_poll(void)
{
  static uint8_t plus=10;
  static uint8_t minus=10;

  if(IN_VBIAS_UP)
  {
   plus=10;
  }
  if(IN_VBIAS_DOWN)
  {
   minus=10;
  }

  if(!plus)
  {
   bias_plus();
   plus=10;
  }

  if(!minus)
  {
   bias_minus();
   minus=10;
  }
}
