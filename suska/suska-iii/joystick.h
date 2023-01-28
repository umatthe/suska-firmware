#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__
#include <stdint.h>

#include "config.h"


void joystick_init(void);
uint8_t joystick_poll(void);
void joystick_sendstate(void);
#ifndef SUSKA_C
void joystick_sendspi(uint8_t val);
#endif
#ifdef JOYENABLE
#define joystick_enable()  JOYENABLEPORT&=~_BV(JOYENABLE)
#define joystick_disable() JOYENABLEPORT|=_BV(JOYENABLE)
#endif
#endif
