/*
 i2a.c
 This is a simple implementation of
 converting float into ascii readable output
 Currently restricted to 1 digit after decimal point and
 4 leading digits, like 9999.9
 The space for the ascii string is allocated locally (8 bytes)

    Stefan Weigl Aug 2007

*/

#include "f2a.h"
#include <avr/io.h>


noinline const char* f2a(float fl)
{
    static char d[F2S1MAXLEN];
    d[F2S1MAXLEN-1] = 0;
    uint8_t* p = &(d[F2S1MAXLEN-1-1]);
    uint8_t neg=0;
    uint32_t i;
    uint8_t rem;
    
    if (fl < 0.0)
    {
        neg = 1;
        fl = -fl;
    }
    
    i = (uint32_t)(fl * 100);
    rem = i%100;

    /* rounding for .x */
    if ((rem %10) > 5) 
       rem = i%100/10+1;
    else
       rem = i%100/10;
    if (rem >= 10) // overflow
    {
        rem = 0;
        i += 100;
    }
    
    *p-- = rem + '0';
    *p-- = '.';
    
    i /= 100;

    /* now itoa needs attention */
    if (i == 0)
    {
        *p = '0';
        return p;
    }

    while (i > 0)
    {
        uint8_t x = i % 10;
        *p-- = '0' + x;
        i /= 10;
    }
    if (neg != 0) *p = '-'; else p++;
    
    return p;
}
