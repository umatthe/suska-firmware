#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "pindefs.h"
#include "ps2.h"
#include "../../sysboot/shared/uart.h"

#ifdef SUSKA_III_C
#define UP		0x01
#define DOWN	0x02
#define LEFT	0x04
#define RIGHT	0x08
#define BUTTONS	0x20
#elif defined SUSKA_III_B
#define UP		0x10
#define DOWN	0x20
#define LEFT	0x40
#define RIGHT	0x80
#endif

static uint8_t joystick_read(void)
{
	return JOY_PIN;
}

void joystick_init(void)
{
#ifdef SUSKA_III_C
	/* set as inputs with weak pullups */
	JOY_DDR = 0;
	JOY_PORT = 0xFF;
#elif defined SUSKA_III_B
	JOY_DDR &= 0x0F;
	//JOY_PORT = 0xF0;
	JOY_ENA_DDR = 1;
	JOY_ENA = 1;
	JOY_BUTTONS_DDR = 0; /* joystick BUTTONS input */
#endif
}

void blink_led(void)
{
#ifdef SUSKA_III_C
	LED_PORT &= ~_BV(LED_PS2_MOUSE_AVAIL);
	_delay_ms(300);
	LED_PORT |= _BV(LED_PS2_MOUSE_AVAIL);
#endif
}

void joystick_poll(void)
{
	static uint8_t state = 0, prev_state = 0xff, joystate;

#ifdef SUSKA_III_B
	JOY_ENA = 0; /* activate joyport for Suska-III-B */
#endif

	state = ~joystick_read();
	if (state != prev_state) {

		/* send header (0xfe - joystick 0, 
		   0xff - joystick 1) and state */
		uputch(0xFF);
		joystate = 0;

#ifdef SUSKA_III_C
		if (state & BUTTONS) {
			joystate |= 0x80;
		}
#elif defined SUSKA_III_B
		if (JOY_BUTTONS) {
			joystate |= 0x80;
		}
#endif

		if (state & UP) {
			joystate |= 0x01;
		}
		if (state & DOWN) {
			joystate |= 0x02;
		}
		if (state & LEFT) {
			joystate |= 0x04;
		}
		if (state & RIGHT) {
			joystate |= 0x08;
		}
		uputch(joystate);
		prev_state = state;
	}
}
