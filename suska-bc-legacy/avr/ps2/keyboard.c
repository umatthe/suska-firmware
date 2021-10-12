/**
 * Driver for a PS/2 keyboard
 *
 *		 /|__|\
 *		/5 [] 6\	
 *		|3	  4|
 *		\1____2/
 *		 
 * 1 keyboard data
 * 2 na
 * 3 grd
 * 4 +5V (300ma)
 * 5 clk
 * 6 na
 * 
 * this ps/2 interface is based on avr appnote 313
 * and http://www.computer-engineering.org/index.php?title=Main_Page
 *
 * very helpful was: http://www.mikrocontroller.net/topic/112361
 * and the asociated source code.
 *
 * data and clk state is high
 * 11 bit frame: 1 start bit, 8 bit data lsb first, 1 bit parity (odd), stop bit
 * 
 * start		lsb	data	msb		parity	  stop
 * 0			x x x x x x x x		x		  1
 * 
 * (c) Jens Carroll, Inventronik GmbH
 */
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "keyboard.h"
#include "timer_overflow.h"
#include "pindefs.h"
#include "ps2.h"
#include "../../sysboot/shared/uart.h"

#define KB_BUFF_SIZE 32
#define FALSE 0
#define TRUE (!FALSE)

static volatile uint8_t edge, bitcount;			 /* edge: 0 = neg.	1 = pos. */
static volatile uint8_t *in_ptr, *out_ptr;
static volatile uint8_t buffcnt;
static uint8_t kb_buffer[KB_BUFF_SIZE];

static void keyboard_put(uint8_t c);
static uint8_t keyboard_send_byte(uint8_t data);
static uint8_t decode_atari(uint8_t sc, uint8_t ext_code);

/*
 * Initialize the PS/2 keyboard
 */
void keyboard_init(void)
{
#if defined(__AVR_ATmega644__)
	EIMSK |= _BV(INT0);	/* enable irpt 0 */
	EICRA |= _BV(ISC01); /* trigger on falling edge of INT0 */
	EICRA &= ~_BV(ISC00);
#else
	GICR |= _BV(INT0);		/* enable irpt 0 */
	MCUCR |= _BV(ISC01);	/* INT0 interrupt on falling edge */
	MCUCR &= ~_BV(ISC00);
#endif

	keyboard_clear_buffer();	/* clear  buffer */
	edge = 0;				/* 0 = falling edge  1 = rising edge */
	bitcount = 11;
}

ISR(INT0_vect)
{
	static uint8_t data;	/* holds the received keyboard data */

	if (!edge) {	/* routine entered at falling edge */
		if (bitcount < 11 && bitcount > 2) {	/* bit 3 to 10 is data. Parity bit, */
																					/* start and stop bits are ignored. */
			data >>= 1;
			if ((KB_DATA_PIN & _BV(KB_DATA)))
				data |=  0x80;	/* store a '1' */
		}

#if defined(__AVR_ATmega644__)
		EICRA |= _BV(ISC00);	/* set interrupt on rising edge */
#else 
		MCUCR |= _BV(ISC00);	/* set interrupt on rising edge */
#endif
		edge = 1;
	} else	{
									/* routine entered at rising edge */
#if defined(__AVR_ATmega644__)
		EICRA &= ~_BV(ISC00);	/* set interrupt on rising edge */
#else
		MCUCR &= ~_BV(ISC00);	/* set interrupt on falling edge */
#endif
		edge = 0;
		bitcount--;
		if (bitcount == 0) {	/* all bits received */
			keyboard_put(data);
			bitcount = 11;
		}
	}
}

/*
 * Clear keyboard fifo
 */
void keyboard_clear_buffer(void)
{
	in_ptr = out_ptr = kb_buffer;
	buffcnt = 0;
}

/*
 * Send one byte to the ps2 device (here keyboard) and wait for
 * an ACK (0xFA)
 */
uint8_t keyboard_send_cmd(uint8_t data)
{
	uint8_t result = FALSE;

	if (keyboard_send_byte(data)) {
		_delay_ms(50);

		/* did we receive ACK before timeout? */
		if (buffcnt!=0 && keyboard_get()==0xFA) {
			result = TRUE;
		}
	}
	return result;
}

/*
 * Send one byte to the ps2 device (here keyboard)
 *
 * returns TRUE if no timeout occurred and the device responds with ACK
 * otherwise FALSE
 */
static uint8_t keyboard_send_byte(uint8_t data)
{
	uint8_t j, result = FALSE, parity = 0;

	if (!t0_overflow())
		return FALSE;		/* send in progress */

#if defined(__AVR_ATmega644__)
	EIMSK &= ~_BV(INT0);	/* disable INT0 */
#else
	GICR &= ~_BV(INT0);	/* disable INT0 */
#endif

	/* KB clock and data as outputs */
	KB_DATA_DDR  |= _BV(KB_DATA);
	KB_CLOCK_DDR |= _BV(KB_CLOCK);

	/*	clock and data to high */
	KB_DATA_PORT  |= _BV(KB_DATA);
	KB_CLOCK_PORT |= _BV(KB_CLOCK);

	/* KB clock now to low */
	KB_CLOCK_PORT &= ~_BV(KB_CLOCK);

	/* minimum delay between clock low and data low */
	_delay_us(120);

	/* next KB data to low */
	KB_DATA_PORT &= ~_BV(KB_DATA);

	/* send start bit (just with this delay) */
	_delay_us(20);

	/* release KB clock as input */
	KB_CLOCK_DDR &= ~_BV(KB_CLOCK);
	_delay_us(50);

	j = 0;
	t0_on_32ms();

	do {
		/* wait until clock gets low */
		while ((KB_CLOCK_PIN & _BV(KB_CLOCK)) && !t0_overflow());

		/* timer0 overflow? */
		if (t0_overflow()) break;

		if (j<8) {

			KB_DATA_PORT = SET_BIT(data, KB_DATA_PORT, KB_DATA);
			if (data & 0x01) {
				parity ^= 0x01;
			}

			data >>= 1;
		} else if (j==8) {
			/* insert parity */
			KB_DATA_PORT = SET_BIT(~parity, KB_DATA_PORT, KB_DATA);
		} else if (j>8) {
			/* KB clock and data as inputs again */
			KB_DATA_DDR &= ~_BV(KB_DATA);
			KB_CLOCK_DDR &= ~_BV(KB_CLOCK);

			if (j==10) {	
				/* receive ACK eventually
				   wait until data gets low (ack from device) */
				while ((KB_DATA_PIN & _BV(KB_DATA)) && !t0_overflow());
				if (!t0_overflow())
					result = TRUE;

				while ((KB_DATA_PIN & _BV(KB_DATA)) && (KB_CLOCK_PIN & _BV(KB_CLOCK)) && !t0_overflow());
				if (t0_overflow())
					result = FALSE;
				break;
			}
		}
		
		/* wait until clock gets high or timeout */
		while ((!(KB_CLOCK_PIN & _BV(KB_CLOCK))) && !t0_overflow());
		if (t0_overflow())
			break;
		j++;
	} while (j<11);

	/* KB clock and data as input */
	KB_DATA_DDR &= ~_BV(KB_DATA);
	KB_CLOCK_DDR &= ~_BV(KB_CLOCK);

#if defined(__AVR_ATmega644__)
	/* clear interrupt flag bit (write a 1) to prevent ISR entry 
	   upon irpt enable */
	EIFR = _BV(INTF0);

	/* enable keyboard irpt */
	EIMSK |= _BV(INT0);
#else
	/* clear interrupt flag bit (write a 1) to prevent ISR entry 
	   upon irpt enable */
	GIFR = _BV(INTF0);

	/* enable keyboard irpt */
	GICR |= _BV(INT0);
#endif

	/* stop timer */
	t0_off();

	return result;
}


static uint8_t decode_atari(uint8_t sc, uint8_t ext_code)
{
	static char c, last_c=0, break_code=0, send_key_code=0, alt_mapped=0, caps_lock=0;
	static char coding[] =
	{ /*  0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f */
/*0*/	  0, 67,  0, 63, 61, 59, 60, 71,  0, 68, 66, 64, 62, 15,255,  0,
/*1*/	  0, 56, 42,  0, 29, 16,  2,  0,  0,  0, 44, 31, 30, 17,  3,  0,
/*2*/	  0, 46, 45, 32, 18,  5,  4,  0,  0, 57, 47, 33, 20, 19,  6,  0,
/*3*/	  0, 49, 48, 35, 34, 21,  7,  0,  0,  0, 50, 36, 22,  8,  9,  0,
/*4*/	  0, 51, 37, 23, 24, 11, 10,  0,  0, 52, 53, 38, 39, 25, 12,  0,
/*5*/	  0,  0, 40,  0, 26, 13,  0,  0, 58, 54, 28, 27,  0, 41,  0,  0,
/*6*/	  0, 96,  0,  0,  0,  0, 14,  0,  0,109,  0,106,103,  0,  0,  0,
/*7*/	112,113,110,107,108,104,  1,  0,  0, 78,111, 74,102,105,  0,  0,
/*8*/	  0,  0,  0, 65,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 };

	static char ext_coding[] =
	{ /*  0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f */
/*0*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
/*1*/	  0, 56, 99,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 98, 
/*2*/	  0,  0,  0,  0,  0,  0,  0,  0, 97,  0,  0,  0,  0,  0,  0,  0, 
/*3*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
/*4*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,101,  0,  0,  0,  0,  0, 
/*5*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,114,  0,  0,  0,  0,  0, 
/*6*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 75,  0,  0,  0,  0, 
/*7*/	 82, 83, 80,  0, 77, 72,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
/*8*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };

/*
//sc -= 12;
uputch(sc);
uputch(sc|0x80);
return 0;
*/

	/* ps2 break code? */
	if (sc != 0xF0) {
		if (ext_code)
			c = ext_coding[sc];
		else
			c = coding[sc];
	} else {
		break_code = 1;
		return ext_code;
	}

	/* ^ */
	if (c == 255) {
		send_key_code = 42;
		c = 41;
	} else {
		send_key_code = 0;
	}

	/* map atari alt keys */
	if (last_c == 56 || alt_mapped) {
		alt_mapped = 1;
		send_key_code = 0;
		switch (c) {
			case 9:  c = 39; break; /* [ */
			case 10: c = 40; break; /* ] */
			case 16: c = 26; break; /* @ */
			case 27: c = 43; break; /* tilde */
			case 8:  c = 39; send_key_code = 42; break; /* { */
			case 11: c = 40; send_key_code = 42; break; /* } */
			case 12: c = 26; send_key_code = 42; break; /* \ */
			case 96: c = 43; send_key_code = 42; break; /* | */
			default:
				alt_mapped = 0;
		}
	}

	/* caps lock led toggle */
	if (c == 58 && !break_code) {
		keyboard_send_cmd(0xED);
		if (!caps_lock)
			keyboard_send_cmd(4);
		else
			keyboard_send_cmd(0);
		caps_lock = !caps_lock;
	}

	/* create break code if necessary */
	if (break_code) {
		c |= 0x80;
	}

	/* ignore typematic repeat */
	if (c != last_c) {
		if (send_key_code>0 && !break_code)
			uputch(send_key_code);
		uputch(c);
		if (send_key_code>0 && break_code)
			uputch(send_key_code | 0x80);
	}
	last_c = c;
	break_code = 0;
	return 0;
}

/*
 * Add a character to the keyboard fifo
 */
static void keyboard_put(uint8_t c)
{
	/* put character into buffer and incr ptr */
	*in_ptr++ = c;
	buffcnt++;

	/* pointer wrapping */
	if (in_ptr >= kb_buffer + KB_BUFF_SIZE)
		in_ptr = kb_buffer;
}

/*
 * Retrieve a character from the keyboard fifo
 */
uint8_t keyboard_get(void)
{
	uint8_t byte;

	while (buffcnt == 0); /* wait for data */
	byte = *out_ptr++;	  /* get byte */

	if (out_ptr >= kb_buffer + KB_BUFF_SIZE) /* pointer wrapping */
		out_ptr = kb_buffer;
	buffcnt--;	/* decrement buffer count */

	return byte;
}

/*
 * Check if a keyboard id available
 */
uint8_t is_keyboard_available(void)
{
	uint8_t i, result, keyboard_ok = FALSE;

	/* try 3 times to get the keyboard id */
	for (i=0; i<3; i++) {
		_delay_ms(100);

		result = keyboard_send_cmd(KB_RESET);
		if (!result) 
			continue;

		/* wait for self-test complete (0xAA) */
		if (keyboard_get() != 0xAA)
			continue;

		result = keyboard_send_cmd(KB_READ_ID);
		if (!result)
			continue;

		/* check for 0xAB, 0x83 */
		if (keyboard_get() == 0xAB) {
			keyboard_get(); /* ignore 0x83 */
			keyboard_ok = TRUE;
			break;
		}
	}
	return keyboard_ok;
}

void keyboard_poll(void)
{
	uint8_t data;
	static uint8_t ext_code=0;

	while (buffcnt > 0) {
		data = keyboard_get();
		/* ext. keys? */
		if (data == 0xE0)
			ext_code = 1;
		else
			ext_code = decode_atari(data, ext_code);
	}
}
