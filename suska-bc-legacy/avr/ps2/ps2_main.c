#include <avr/pgmspace.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "pindefs.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "ps2.h"
#include "timer_overflow.h"
#include "../../sysboot/shared/uart.h"
#include "bias.h"

#define PS2_MOUSE
#define PS2_KEYBOARD
#define JOYSTICK

//#define UMA
#ifdef UMA
#define DEBUG_LED_DDR           SBIT(DDRC,  6)
#define DEBUG_LED               SBIT(PORTC, 6)
#endif

void command_poll( void)
{
	uint8_t command=0,command1=0;
        if(!uart_receive_no_wait(&command,1))
	{
		switch(command)
		{
			case 0x16: //Joystick interrogation --> Reply Joystick Status-Paket
				// only a dummy to make GFA-Basic happy
				uputch(0xFF);
				uputch(0);
				break;

			case 0x80: //RESET --> Reply 0xF1 (Keyboard ok)
				uputch(0xF1);
				break;

                        // Not implemented Commands

			case 0x07: //Mouse
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
			case 0x0c:
			case 0x0d: // interrgate mouse pos --> Reply Mouse-Pos
			case 0x0e:
			case 0x0f:
			case 0x10:
			case 0x11: //Resume
			case 0x12: //disable Mouse
			case 0x13: //Pause
			// Joystick
			case 0x14:
			case 0x15:
			case 0x17:
			case 0x18:
			case 0x19:
			case 0x1a:
			// Clock
			case 0x1b: // Time-of-day set
			case 0x1c: // Time-of-day get --> Reply ToD
		        //  Status für alle sets --> Reply status
			case 0x87: 
			case 0x88:
			case 0x89:
			case 0x8a:
			case 0x8b:
			case 0x8c:
			case 0x8d: // gibt's nicht
			case 0x8e:
			case 0x8f:
			case 0x90:
			case 0x91:
			case 0x92:
			case 0x93: // gibt's nicht
			case 0x94:
			case 0x95:
			case 0x96: // gibt's nicht
			case 0x97: // gibt's nicht
			case 0x98: // gibt's nicht
			case 0x99:
			case 0x9a:
#ifdef UMA
				DEBUG_LED=1;
#endif
				break;
			default:
				break;
		}

	}
}


uint8_t mouse_id = 0xFF, keyboard_id = 0xFF;

void ps2_init(void)
{
	timer_init();	/* setup timer 0 and timer 2 overflow irpts */
#ifdef JOYSTICK
	joystick_init(); /* initialize port pins for the joystick */
#endif
#ifdef PS2_KEYBOARD
  keyboard_init(); /* initialize a ps/2 keyboard */
#endif
#ifdef PS2_MOUSE
  mouse_init();	/* initialize a ps/2 mouse */
#endif	

#ifdef SUSKA_III_C
	LED_PORT = 0xFF;
	LED_DDR = _BV(LED_PS2_KEYBOARD_AVAIL) | _BV(LED_PS2_MOUSE_AVAIL);
#endif
#ifdef UMA
	DEBUG_LED_DDR=1;
	DEBUG_LED = 0;
#endif
}

static uint8_t ps2_mouse_init(void)
{
	uint8_t retval = 0;
#ifdef PS2_MOUSE
	/* reset the mouse and check for FA, AA, mouse-id */
	if (is_mouse_available()) {

#ifdef DEBUG
		uputs("PS/2 mouse detected\r\n");
#endif
		retval++;

		/* set mouse sample rate */
		mouse_send_cmd(MS_SET_SAMPLE_RATE);
		mouse_send_cmd(100);

		/* set mouse resolution (1count/mm) */
		mouse_send_cmd(MS_SET_RESOLUTION);
		mouse_send_cmd(2);

		mouse_send_cmd(MS_GET_DEVICE_ID);
		mouse_id = mouse_get(); /* 0 = PS/2 mouse, 3 = Microsoft Intellimouse */

		mouse_send_cmd(MS_ENABLE_DATA_REPORTING);

		mouse_clear_buffer();
#ifdef SUSKA_III_C
		LED_PORT &= ~_BV(LED_PS2_MOUSE_AVAIL);
#endif
	} else {
#ifdef DEBUG
		uputs("No PS/2 mouse detected\r\n");
#endif
	}
#endif
	return retval;
}

static uint8_t ps2_keyboard_init(void)
{
	uint8_t retval = 0;
#ifdef PS2_KEYBOARD
	if (is_keyboard_available()) {
#ifdef DEBUG
		uputs("PS/2 keyboard detected\r\n");
#endif
		retval++;

		/* read keyboard id */
		keyboard_send_cmd(KB_READ_ID);
		keyboard_id = keyboard_get();

		/* keyboard enable */
		keyboard_send_cmd(KB_ENABLE);

		/* set typematic rate/delay */
		keyboard_send_cmd(KB_TYPEMATIC_RATE_DELAY);
		keyboard_send_cmd(0); /* 250ms / 30.0 reports/sec */

		keyboard_clear_buffer();
#ifdef SUSKA_III_C
		LED_PORT &= ~_BV(LED_PS2_KEYBOARD_AVAIL);
#endif
	} else {
#ifdef DEBUG
		uputs("No PS/2 keyboard detected\r\n");
#endif
	}
#endif
	return retval;
}

int main(void)
{
	uint8_t mouse_detected = 0, keyboard_detected = 0;

	_delay_ms(3000);

#ifdef SUSKA_III_B
	/* set data direction register and outputs */
	OUT_RESET_CORE = OUT_RESET = 0;
	OUT_RESET_CORE_DDR = OUT_RESET_DDR = 1; /* output */
	OUT_PS_EN = 1;
	OUT_PS_EN_DDR = 1; /* output */
	OUT_BOOT_ACK = 0;
	OUT_BOOT_ACK_DDR = 1; /* output */
#endif
#ifdef SUSKA_III_C
        bias_init();
#endif

#if 0
	_delay_ms(2000);
	DEBUG_LED_DDR = 1; /* output */
	DEBUG_LED = 1;
  _delay_ms(1000);
  DEBUG_LED = 0;
  _delay_ms(2000);
#endif

	uart_init(PS2_BAUDRATE);
	uart_set_timeout(200); /* set to 200ms */

	ps2_init();
	sei();			/* enable interrupts */

	mouse_detected += ps2_mouse_init();
	keyboard_detected += ps2_keyboard_init();

	if(keyboard_detected)
	{
	   while (1) {
#ifdef PS2_MOUSE
		if (mouse_detected > 0)
			mouse_poll();
#endif
#ifdef PS2_KEYBOARD
		if (keyboard_detected > 0)
		{
			keyboard_poll();
		}
#ifdef JOYSTICK
			joystick_poll();
#endif
			command_poll();
#ifdef SUSKA_III_C
                        bias_poll();
#endif
#endif
	   }
	}
	else
	{
		/*  ps2 keyboard not detected */
		UCSRB = 0;
		for(;;);
	}
}
