/**
 * Driver for a PS/2 mouse
 *
 *		 /|__|\
 *		/5 [] 6\	
 *		|3	  4|
 *		\1____2/
 *		 
 * 1 mouse data
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
 * and the associated source code.
 *
 * data and clk state is high
 * 11 bit frame: 1 start bit, 8 bit data lsb first, 1 bit parity (odd), stop bit
 * 
 * start		lsb	data	msb		parity	  stop
 * 0			x x x x x x x x		x		  1
 * 
 * (c) Jens Carroll, 2007, 2015 Inventronik GmbH
 */
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "mouse.h"
#include "timer_overflow.h"
#include "pindefs.h"
#include "ps2.h"
#include "../../sysboot/shared/uart.h"

#define MS_BUFF_SIZE 32
#define FALSE 0
#define TRUE (!FALSE)

static volatile uint8_t edge, bitcount;			 /* edge: 0 = neg.	1 = pos. */
static volatile uint8_t *in_ptr, *out_ptr;
static volatile uint8_t buffcnt;
static uint8_t ms_buffer[MS_BUFF_SIZE];

static void mouse_put(uint8_t c);
static uint8_t mouse_send_byte(uint8_t data);

/*
 * Initialize the PS/2 mouse
 */
void mouse_init(void)
{
#if defined(__AVR_ATmega644__)
	EIMSK |= _BV(INT1);	/* enable irpt 1 */
	EICRA |= _BV(ISC11); /* trigger on falling edge of INT1 */
	EICRA &= ~_BV(ISC10);
#else
	GICR = _BV(INT1);		/* enable irpt 1 */
	MCUCR |= _BV(ISC11);	/* INT1 interrupt on falling edge */
	MCUCR &= ~_BV(ISC10);
#endif

	mouse_clear_buffer();	/* clear  buffer */
	edge = 0;	/* 0 = falling edge, 1 = rising edge */
	bitcount = 11;
}

ISR(INT1_vect)
{
	static uint8_t data; /* holds the received mouse data */

	if (!edge) { /* routine entered at falling edge */
		if (bitcount < 11 && bitcount > 2) { /* bit 3 to 10 is data. Parity bit, */
			/* start and stop bits are ignored. */
			data >>= 1;
			if ((MS_DATA_PIN & _BV(MS_DATA)))
				data |= 0x80;	/* store a '1' */
		}
#if defined(__AVR_ATmega644__)
		EICRA |= _BV(ISC10); /* set interrupt on rising edge */
#else
		MCUCR |= _BV(ISC10); /* set interrupt on rising edge */
#endif
		edge = 1;
	} else	{	/* routine entered at rising edge */
#if defined(__AVR_ATmega644__)
		EICRA &= ~_BV(ISC10);	/* set interrupt on falling edge */
#else
		MCUCR &= ~_BV(ISC10);	/* set interrupt on falling edge */
#endif
		edge = 0;
		bitcount--;
		if (bitcount == 0) {	/* all bits received */
			mouse_put(data);		/* Add data to buffer */
			bitcount = 11;
		}
	}
}

/*
 * Clear mouse fifo
 */
void mouse_clear_buffer(void)
{
	in_ptr = out_ptr = ms_buffer;
	buffcnt = 0;
}

/*
 * Send one byte to the ps2 device (here mouse) and wait for
 * an ACK (0xFA)
 */
uint8_t mouse_send_cmd(uint8_t data)
{
	uint8_t result = FALSE;

	if (mouse_send_byte(data)) {
		_delay_ms(50);

		/* did we receive ACK before timeout? */
		if (buffcnt!=0 && mouse_get()==0xFA) {
			result = TRUE;
		}
	}
	return result;
}

/*
 * Send one byte to the ps2 device (here mouse)
 *
 * returns TRUE if no timeout occurred and the device responds with ACK
 * otherwise FALSE
 */
static uint8_t mouse_send_byte(uint8_t data)
{
	uint8_t j, result = FALSE, parity = 0;

	if (!t2_overflow())
		return FALSE;		/* send in progress */

#if defined(__AVR_ATmega644__)
	EIMSK &= ~_BV(INT1);	/* disable INT1 */
#else
	GICR &= ~_BV(INT1);	/* disable INT1 */
#endif

	/* MS clock and data to high */
	MS_DATA_PORT  |= _BV(MS_DATA);
	MS_CLOCK_PORT |= _BV(MS_CLOCK);

	/* MS clock and data as outputs */
	MS_DATA_DDR  |= _BV(MS_DATA);
	MS_CLOCK_DDR |= _BV(MS_CLOCK);

	/* MS clock now to low */
	MS_CLOCK_PORT &= ~_BV(MS_CLOCK);

	/* minimum delay between clock low and data low */
	_delay_us(120);

	/* next MS data to low */
	MS_DATA_PORT &= ~_BV(MS_DATA);

	/* send start bit (just with this delay) */
	_delay_us(20);

	/* release MS clock as input */
	MS_CLOCK_DDR &= ~_BV(MS_CLOCK);
	_delay_us(50);

#if 0
	/* minimum delay between clock low and data low */
	_delay_us(120);

	/* next MS data to low */
	MS_DATA_PORT &= ~_BV(MS_DATA);

	/* send start bit (just with this delay) */
	//_delay_us(20);

	/* release MS clock as input */
	MS_CLOCK_DDR &= ~_BV(MS_CLOCK);

	/* send start bit (just with this delay) */
	_delay_us(20);
#endif

	j = 0;
	t2_on_32ms();

	do {
		/* wait until clock gets low */
		while ((MS_CLOCK_PIN & _BV(MS_CLOCK)) && !t2_overflow());

		//_delay_us(15);

		/* timer2 overflow? */
		if (t2_overflow()) break;

		if (j<8) {

			MS_DATA_PORT = SET_BIT(data, MS_DATA_PORT, MS_DATA);
			if (data & 0x01) {
				parity ^= 0x01;
			}

			data >>= 1;
		} else if (j==8) {
			/* insert parity */
			MS_DATA_PORT = SET_BIT(~parity, MS_DATA_PORT, MS_DATA);
		} else if (j>8) {
			/* MS clock and data as inputs again */
			MS_DATA_DDR &= ~_BV(MS_DATA);
			MS_CLOCK_DDR &= ~_BV(MS_CLOCK);
			_delay_us(10);

			if (j==10) {	
				/* receive ACK eventually
				   wait until data gets low (ack from device) */
				while ((MS_DATA_PIN & _BV(MS_DATA)) && !t2_overflow());
				if (!t2_overflow())
					result = TRUE;

				while ((MS_DATA_PIN & _BV(MS_DATA)) && (MS_CLOCK_PIN & _BV(MS_CLOCK)) && !t2_overflow());
				if (t2_overflow())
					result = FALSE;
				break;
			}
		}
		
		/* wait until clock gets high or timeout */
		while ((!(MS_CLOCK_PIN & _BV(MS_CLOCK))) && !t2_overflow());
		if (t2_overflow())
			break;
		j++;
	} while (j<11);

	/* MS clock and data as input */
	MS_DATA_DDR &= ~_BV(MS_DATA);
	MS_CLOCK_DDR &= ~_BV(MS_CLOCK);


#if defined(__AVR_ATmega644__)
	/* clear interrupt flag bit (write a 1) to prevent ISR entry 
	   upon irpt enable */
	EIFR = _BV(INTF1);

	/* enable mouse irpt */
	EIMSK |= _BV(INT1);
#else
	/* clear interrupt flag bit (write a 1) to prevent ISR entry 
	   upon irpt enable */
	GIFR = _BV(INTF1);

	/* enable mouse irpt */
	GICR |= _BV(INT1);
#endif

	/* stop timer */
	t2_off();

	return result;
}

static void mouse_put(uint8_t c)
{
	/* put character into buffer and incr ptr */
	*in_ptr++ = c;
	buffcnt++;

	/* pointer wrapping */
	if (in_ptr >= ms_buffer + MS_BUFF_SIZE)
		in_ptr = ms_buffer;
}

uint8_t mouse_get(void)
{
	uint8_t byte;

	while (buffcnt == 0); /* wait for data */
	byte = *out_ptr++;	  /* get byte */

	if (out_ptr >= ms_buffer + MS_BUFF_SIZE) /* pointer wrapping */
		out_ptr = ms_buffer;
	buffcnt--;	/* decrement buffer count */

	return byte;
}

uint8_t is_mouse_available(void)
{
	uint8_t i, result, mouse_ok = FALSE;

	/* try 5 times to initialize (reset) the mouse */
	for (i=0; i<3; i++) {
		_delay_ms(100);

		result = mouse_send_cmd(MS_RESET);
		if (result==0)
			continue;

		_delay_ms(5);

		/* check for self-test */
		if (mouse_get() == 0xAA) {
			mouse_get(); /* we don't care about the mouse-id yet */
			mouse_ok = TRUE;
			break;
		}
	}
	return mouse_ok;
}

void mouse_poll(void)
{
	extern uint8_t mouse_id;
	uint8_t i, m[4], pkg_size, header;

	if (mouse_id != 0xFF) {
		/* Microsoft Intellimouse sends 4 bytes */
		pkg_size = (mouse_id==0 ? 3 : 4);

		if (buffcnt < pkg_size)
			return; /* not enough */
#if 0
uputs("\r\n*");
uputx(buffcnt);
uputs("*");
pkg_size=buffcnt;
		for(i=0; i<pkg_size; i++)
{ uputx(mouse_get()); uputs(" "); }
uputs("#");
#endif

		for(i=0; i<pkg_size; i++) {
			m[i] = mouse_get();
		}

		switch(m[0] & 0x7) {
			case 2: header = 0xf9; break; /* relative mouse, button 1 */
			case 1: header = 0xfa; break; /* relative mouse, button 2 */
			case 3: header = 0xfb; break; /* relative mouse, button 1+2 */
			default:
				header = 0xf8; /* relative mouse no buttons*/
		}
//cli();
		uputch(header);
		uputch(m[1]);
		uputch(-m[2]);
//sei();
	}
}
