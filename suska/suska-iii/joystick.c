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
#ifdef JOY0
        JOY0UPDDR&=~_BV(JOY0UP);
        JOY0DOWNDDR&=~_BV(JOY0DOWN);
        JOY0LEFTDDR&=~_BV(JOY0LEFT);
        JOY0RIGHTDDR&=~_BV(JOY0RIGHT);
        JOY0BUTTONDDR&=~_BV(JOY0BUTTON);
#endif
#if defined _PULL_UP_
        JOYUPPORT|=_BV(JOYUP); 
        JOYDOWNPORT|=_BV(JOYDOWN); 
        JOYLEFTPORT|=_BV(JOYLEFT); 
        JOYRIGHTPORT|=_BV(JOYRIGHT); 
        JOYBUTTONPORT|=_BV(JOYBUTTON); 
#ifdef JOY0
        JOY0UPPORT|=_BV(JOY0UP);
        JOY0DOWNPORT|=_BV(JOY0DOWN);
        JOY0LEFTPORT|=_BV(JOY0LEFT);
        JOY0RIGHTPORT|=_BV(JOY0RIGHT);
        JOY0BUTTONPORT|=_BV(JOY0BUTTON);
#endif
#endif
#if defined JOYENABLE
        JOYENABLEDDR|=_BV(JOYENABLE);
        joystick_disable();
#endif
#ifdef SUART_TXD
        softuart_init();
#endif
}
#ifdef JOY0
static uint8_t joystick0_read(void)
{
	uint8_t ret=0;

        ret|=JOY0UPPIN&_BV(JOY0UP)?0:1;
        ret|=JOY0DOWNPIN&_BV(JOY0DOWN)?0:2;
        ret|=JOY0LEFTPIN&_BV(JOY0LEFT)?0:4;
        ret|=JOY0RIGHTPIN&_BV(JOY0RIGHT)?0:8;
        ret|=JOY0BUTTONPIN&_BV(JOY0BUTTON)?0:128;

        return ret;
}

static uint8_t prev_state0 = 0xff;
static void joystick0_sendstate(void)
{
                /* send header (0xfe - joystick 0,
                   0xff - joystick 1) and state */
#ifdef SUART_TXD
                softuart_putc(0xFE);
                softuart_putc(prev_state0);
#else
                uart_putc(0xFE);
                uart_putc(prev_state0);
#endif
#ifdef JOYSPI
//UMA todo Handling of Arcade Joystick0 missing
                if(coretype==CT_ARCADE)		joystick_sendspi(prev_state0);
#endif
}
#endif //JOY0

static uint8_t prev_state = 0xff;
uint8_t joystick_poll( void )
{
 uint8_t state;
#if defined JOY0
 uint8_t state0;
#endif
#if defined JOYENABLE
        joystick_enable();
#endif
        state = joystick_read();
#if defined JOY0
        state0 = joystick0_read();
#endif
#if defined JOYENABLE
        joystick_disable();
#endif
        if (state != prev_state) 
        {
                prev_state = state;
		joystick_sendstate();
	}
#if defined JOY0
        if (state0 != prev_state0) 
        {
                prev_state0 = state0;
		joystick0_sendstate();
	}
#endif
return prev_state; // UMA todo return value only used for debugging - joystick0 missing
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
