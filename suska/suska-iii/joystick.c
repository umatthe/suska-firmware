#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include "coretype/coretypes.h"

#include "config.h"
#ifdef SUART_TXD
#include "../softuart/softuart.h"
#else
#include "../uart-irq/uart-irq.h"
#endif

#ifdef SUSKA_C
#error "No Joystick at Suska-C SDC"
#else
#include "joystick.h"
#include "suskaspi.h"
#endif

static uint8_t joystick_read(void)
{
	uint8_t ret=0;

        ret|=JOYUPPIN&_BV(JOYUP)?0:1; 
        ret|=JOYDOWNPIN&_BV(JOYDOWN)?0:2; 
        ret|=JOYLEFTPIN&_BV(JOYLEFT)?0:4; 
        ret|=JOYRIGHTPIN&_BV(JOYRIGHT)?0:8; 
        ret|=JOYBUTTONPIN&_BV(JOYBUTTON)?0:128; 
        
        return ret;
}

void joystick_init(void)
{
        JOYUPDDR&=~_BV(JOYUP);
        JOYDOWNDDR&=~_BV(JOYDOWN);
        JOYLEFTDDR&=~_BV(JOYLEFT);
        JOYRIGHTDDR&=~_BV(JOYRIGHT);
        JOYBUTTONDDR&=~_BV(JOYBUTTON);
#if defined _PULL_UP_
        JOYUPPORT|=_BV(JOYUP); 
        JOYDOWNPORT|=_BV(JOYDOWN); 
        JOYLEFTPORT|=_BV(JOYLEFT); 
        JOYRIGHTPORT|=_BV(JOYRIGHT); 
        JOYBUTTONPORT|=_BV(JOYBUTTON); 
#endif
#if defined JOYENABLE
        JOYENABLEDDR|=_BV(JOYENABLE);
        joystick_disable();
#endif
#ifdef SUART_TXD
        softuart_init();
#endif
}

static uint8_t prev_state = 0xff;
void joystick_poll( void )
{
 uint8_t state;
#if defined JOYENABLE
        joystick_enable();
#endif
        state = joystick_read();
#if defined JOYENABLE
        joystick_disable();
#endif
        if (state != prev_state) 
        {
                prev_state = state;
		joystick_sendstate();
	}
}

void joystick_sendstate(void)
{
                /* send header (0xfe - joystick 0,
                   0xff - joystick 1) and state */
#ifdef SUART_TXD
                softuart_putc(0xFF);
                softuart_putc(prev_state);
#else
                uart_putc(0xFF);
                uart_putc(prev_state);
//		uart_puts_P("Joy : ");
//  		uart_puthexbyte(prev_state);
//		uart_eol();
#endif
#ifdef JOYSPI
if(coretype==CT_ARCADE)		joystick_sendspi(prev_state);
#endif
}
#ifdef JOYSPI
void joystick_sendspi(uint8_t val)
{
        SS_ENABLEJOY;
        Suskaspi_send(val);
        SS_DISABLE;
//	_delay_ms(20);
}
#endif
