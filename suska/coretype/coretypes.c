#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "../uart-irq/uart-irq.h"

#include "config.h"
#include "coretypes.h"

uint8_t coretype    = 0xAF;
uint8_t coresubtype = 0xFE;

void show_suska_subtype( void )
{
                    switch(coresubtype)
                    {
                     case WF_030L:
                       uart_puts_P("WF68K30L");
                       break;
                     case WF_030:
                       uart_puts_P("WF68K30");
                       break;
                     case WF_010:
                       uart_puts_P("WF68K10");
                       break;
                     case WF_000:
                       uart_puts_P("WF68K00");
                       break;
                     default:
                       uart_puts_P("unknown");
                       break;
                     }
}
