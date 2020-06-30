/**
 * Driver for a PS/2 keyboard
 *
 *		 /|__|\
 *		/5 [] 6\
 *		|3    4|
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
 * 2019/07/10 Udo Matthe: Adapt to Suska-III-BF
 */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "keyboard.h"
#include "mouse.h"
#include "timer_overflow.h"
#include "../uart-irq/uart-irq.h"

#include "config.h"
#include "../shell/shell.h"

extern volatile uint8_t kb_bc;
extern volatile uint8_t *in_ptr, *out_ptr;
extern volatile uint8_t *msin_ptr, *msout_ptr;

static volatile uint8_t kb_bitcount;
static volatile uint8_t ms_bitcount;

uint8_t kb_available=0;
uint8_t ms_available=0;

/*
 * Initialize the PS/2 Stuff
 */
void ps2_init(void)
{
	keyboard_clear_buffer();	/* clear  buffer */
	mouse_clear_buffer();	/* clear  buffer */
	kb_bitcount = 11;
	ms_bitcount = 11;
	kb_bc=0;

	EIMSK|= _BV(PCIE1);
	//PCINT15=KB PCINT14=MS
	PCMSK1|=_BV(PCINT15)|_BV(PCINT14);
}

void ps2_dev_init(void)
{
        kb_available = is_keyboard_available();
        ms_available = is_mouse_available();
	if(ms_available)
	{
		mouse_init();
	}
}


ISR(PCINT1_vect)
{
        static uint8_t kbdata;    /* holds the received keyboard data */
        static uint8_t msdata;    /* holds the received mouse data */
	static uint8_t pins=_BV(KB_CLOCK)|_BV(MS_CLOCK);
	uint8_t kbi;
	uint8_t msi;
	
	if((KB_CLOCK_PIN & _BV(KB_CLOCK))!= (pins &_BV(KB_CLOCK))) kbi=1; else kbi=0;
	if((KB_CLOCK_PIN & _BV(MS_CLOCK))!= (pins &_BV(MS_CLOCK))) msi=1; else msi=0;
	pins=KB_CLOCK_PIN;

	if (kbi & !(pins & _BV(KB_CLOCK)))
	{
		if (kb_bitcount < 11 && kb_bitcount > 2) {    /* bit 3 to 10 is data. Parity bit, */
			/* start and stop bits are ignored. */
			kbdata >>= 1;
			if ((KB_DATA_PIN & _BV(KB_DATA)))
                                kbdata |=  0x80;  /* store a '1' */
		}

		kb_bitcount--;
		if (kb_bitcount == 0) 
		{    /* all bits received */
			keyboard_put(kbdata);
			kb_bitcount = 11;
		}
	}
	if (msi & !(pins & _BV(MS_CLOCK)))
	{
		if (ms_bitcount < 11 && ms_bitcount > 2) {    /* bit 3 to 10 is data. Parity bit, */
			/* start and stop bits are ignored. */
			msdata >>= 1;
			if ((MS_DATA_PIN & _BV(MS_DATA)))
                                msdata |=  0x80;  /* store a '1' */
		}

		ms_bitcount--;
		if (ms_bitcount == 0) 
		{    /* all bits received */
			mouse_put(msdata);
			ms_bitcount = 11;
		}
	}
	kb_bc++;
}
