/***
 * Bootloader for programming the Suska FLASH-Proms through the FPGA.
 * The program also handles the Suska buttons to activate the FPGA bootloader.
 *
 * Program to receive and transmit a stream of characters over RS232/USB
 * The program secures the data transmission with REQ, ACK and a CRC 
 *
 * Copyright (c) 2007, 2008, 2015 Inventronik GmbH
 * All rights reserved.
 *
 * ------------------------------------------------------------------------
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the Inventronik nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
//#define UMA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "portdef.h"

#include "../../sysboot/shared/comprot.h"
#include "../../sysboot/shared/uart.h"

#ifdef SUSKA_III_B
#define PS2
#include "../ps2/joystick.h"
#include "../ps2/keyboard.h"
#include "../ps2/mouse.h"
#include "../ps2/ps2.h"
#include "../ps2/pindefs.h"
#elif defined UMA
#define DEBUG_LED_DDR           SBIT(DDRA,  3)
#define DEBUG_LED               SBIT(PORTA, 3)
#endif

#define soft_reset()        \
do                          \
{                           \
  wdt_enable(WDTO_15MS);    \
  for(;;);                  \
} while(0)

#define FALSE 0
#define TRUE !FALSE

#define MODE_ERASE_WRITE	0x23
#define MODE_WRITE	  		0x20
#define SYSBOOT_BAUDRATE  38400

/* Needed by avr-gcc stdio */
extern FILE mystdout;
uint8_t rcvbuf[PKG_SIZE];

/* Constants */
const char version[]="0.2\n";

/* Global vars */
volatile uint16_t flash_start_addr = 0x0;
volatile uint8_t mode = MODE_ERASE_WRITE;

#define FLASH_MAX_SIZE		(4*1024*1024UL)	/* 4 MB Flash */
volatile uint32_t flash_read_idx = 0;

volatile uint8_t power_on = FALSE;
volatile uint8_t reset_on = FALSE;
volatile uint8_t reset_core_on = FALSE;

enum flash_states {
	FLASH_WAIT_FOR_ADDR_REQ,
	FLASH_SEND_MODE,
	FALSH_WAIT_DATA,
	FLASH_WRITE_SECTION,
	FLASH_READ_SECTION,
	FLASH_DONE
};

volatile enum flash_states fstate = FLASH_WAIT_FOR_ADDR_REQ;

#ifdef SUSKA_III_B
/**
 * Initialize WDT. Please switch wdt on with fuses
 * see: http://www.avrfreaks.net/forum/atmega644pa-wdt-bootrst
 */
//void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
__attribute__((naked,section(".init3")))
void wdt_init(void)
{
	cli();
	wdt_reset();
	MCUSR &= ~(1<<WDRF);
	WDTCSR |= (1<<WDCE) | (1 << WDE);
	WDTCSR = 0;
}
#endif

/**
 * Init port pins (after reset all port pins are inputs)
 */
void port_init(void)
{
	/* set data direction register and outputs */
	OUT_RESET_CORE = OUT_RESET = 0;
	OUT_RESET_CORE_DDR = OUT_RESET_DDR = 1; /* output */

#ifdef SUSKA_III_C
	OUT_PS_50 = OUT_PS_33 = OUT_PS_12 = OUT_PS_VBIAS = 0;
	OUT_PS_50_DDR = OUT_PS_33_DDR = OUT_PS_12_DDR = OUT_PS_VBIAS_DDR = 1; /* output */
#elif defined SUSKA_III_B
	OUT_PS_EN = 0;
	OUT_PS_EN_DDR = 1; /* output */
#endif

	OUT_BOOT_ACK = 0;
	OUT_BOOT_ACK_DDR = 1; /* output */
#ifdef SUSKA_III_C
	OUT_SDRAM_CLKE = 0; /* initialy set to 0, needs to be 1 20us after core reset */
	OUT_SDRAM_CLKE_DDR = 1;
	OUT_UART_EXT = 1;
	OUT_UART_EXT_DDR = 1;
#endif
}

static void spi_enable(uint8_t enable)
{
	if (enable) {
	  /* Enable SPI, Master, MSB first, clock idle low (CPOL=1 - set), sample on
		   falling edge (CPHA=0 - not set), clock rate fck/16 (see Wikipedia) */
		SPI_CLK_DDR = SPI_MOSI_DDR = 1;
		SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0) | _BV(CPOL); /* _BV(CPHA); */
	} else {
		/* Disable SPI*/
		SPI_CLK_DDR = SPI_MOSI_DDR = 0;
	}
}

/**
 * SPI master sends to the slave
 */
void spi_master_send(uint8_t data)
{
    /* start transmission */
	SPDR = data;
		
	/* wait for transmission complete */
	while (!(SPSR & _BV(SPIF)));
}

/**
 * SPI master sends a dummy while receiving at the
 * same time from the slave
 */
uint8_t spi_master_rcv(void)
{
  /* start transmission of a dummy 
	   (at the same time we fill the SPDR with the slave content) */
	SPDR = 0xff;

	/* wait for transmission complete */
	while (!(SPSR & _BV(SPIF)));

	return SPDR;
}

/**
 * Signal ACK to the FPGA
 */
void signal_ack(void)
{
	OUT_BOOT_ACK = 1;
	_delay_us(1);
	OUT_BOOT_ACK = 0;
}

/**
 * SPI master sends address info to the FPGA
 */
void flash_set_addr(uint16_t addr)
{
	uint8_t i;

	if (fstate != FLASH_WAIT_FOR_ADDR_REQ)
		return; /* wrong state */

	spi_master_send(addr << 8); /* msb of a 19bit start addr (bit 19..12) */
	spi_master_send(addr & 0xff); /* lsb of a 19bit start addr (bit 11..4) */

	for (i=0; i<100; i++)
		_delay_ms(40); /* a little pause to get a visual feedback 
						        * seeing a different led flashing frequency */
	signal_ack();
	fstate = FLASH_SEND_MODE;
}

/**
 * SPI master sends mode info (0x20-write, 0x23-erase/write)to the FPGA
 */
void flash_set_mode(void)
{
	if (fstate != FLASH_SEND_MODE)
		return; /* wrong state */

	spi_master_send(mode);
	_delay_ms(2000); /* indicate end of mode section */
	signal_ack();

	fstate = FALSH_WAIT_DATA; /* allow to receive the first flash data package */
}


/**
 * SPI master sends flash data to the FPGA
 */
void flash_program(uint8_t *data, uint16_t size, uint8_t last)
{
	uint16_t i;

	if (fstate != FLASH_WRITE_SECTION || size%2)
		return; /* wrong state or wrong size */

	for (i=0; i<size; i+=2) {
		while (!IN_BOOT_REQ); /* wait for FPGA request */
		spi_master_send((uint8_t) data[i]);
		spi_master_send((uint8_t) data[i+1]);
		signal_ack();
	}

	if (last) {
		_delay_ms(500); /* indicate end of write section */
		fstate = FLASH_READ_SECTION;
	}
}

/**
 * Switch power on/off
 */
uint8_t switch_power(uint8_t on)
{
        uint8_t delcount = 0;
	uint8_t result = FALSE;

	if (power_on != on) {
#ifdef SUSKA_III_C
		OUT_PS_50 = OUT_PS_33 = OUT_PS_12 = OUT_PS_VBIAS = (on ? 1 : 0);
#elif defined SUSKA_III_B
		OUT_PS_EN = (on ? 1 : 0);
#endif

		while (FPGA_CONFIG_DONE && on)
                {
                  _delay_ms(100);
                  if(delcount>10) break;
                  delcount++;

                }
		while ((!FPGA_CONFIG_DONE) && on);

//UMA		while (FPGA_CONFIG_DONE && on)
//UMA			;
		if (on) {
			result = TRUE;
//UMA			_delay_ms(2000); //UMA Hack, da Config_done nicht wirklich config_done ist
		}
		power_on = on;
	}
	return result;
}

/**
 * Set reset pin
 */
void set_reset_pin(uint8_t on)
{
	if (reset_on != on) {
		OUT_RESET = on ? 1 : 0;
		reset_on = on;
	}
}

/**
 * Set reset core pin
 */
void set_reset_core_pin(uint8_t on)
{
	if (reset_core_on != on) {
		OUT_RESET_CORE = on ? 1 : 0;
#ifdef SUSKA_III_C
		OUT_SDRAM_CLKE = on ? 0 : 1;
#endif
		reset_core_on = on;
	}
}

/**
 * Parses command with parameter (returns the parameter as a string)
 */
static char *retrieve_cmd_param(data_container_t *dc, char *param)
{
	uint8_t size = dc->size > 8 ? 8 : dc->size;

	strncpy(param, (char *)&dc->data[1], size-1);
	param[size] = '\n';
	param[size+1] = '\0';
	return param;
}

/**
 * Execute host commands 
 *
 * This controls the spi programming interface such as
 * waiting for activating the Suska bootloader through pressing buttons,
 * returning a version string or checking the usage of the right mcu firmware
 */
char *execute_cmd(data_container_t *dc, char *retstr)
{
	char param[10];

	retstr[0] = '\0';
	if (dc->size>0) {
		switch (dc->data[0]) {
			case 'V': /* version -> return version string */
				retstr[0] = 'V';
				strcpy(&retstr[1], version);
				break;
			case 'W': /* keep in wait state until boot_req and fpga_config_done -> return 'w' 
                 * else bootloader is ready return 'r' */
				strcpy(retstr, (IN_BOOT_REQ && FPGA_CONFIG_DONE) ? "Wr" : "Ww");
				break;
			case 'A': /* address (include address in hex) -> return 'a' */
				flash_start_addr = strtol(retrieve_cmd_param(dc, param), NULL, 16);
				strcpy(retstr, "A");
				break;
			case 'M': /* mode, valid: n - write without erase */
				mode = *retrieve_cmd_param(dc, param) == 'n' ? MODE_WRITE : MODE_ERASE_WRITE;
				strcpy(retstr, "M");
				break;
			default:
				;
		}
	}
	return retstr;
}

uint8_t mouse_id = 0xFF, keyboard_id = 0xFF;
#ifdef PS2
static void ps2_init(void)
{
	timer_init();	/* setup timer 0 and timer 2 overflow irpts */

	joystick_init(); /* initialize port pins for the joystick */
  keyboard_init(); /* initialize a ps/2 keyboard */
  mouse_init();	/* initialize a ps/2 mouse */
}

static uint8_t ps2_mouse_init(void)
{
	uint8_t retval = 0;
	/* reset the mouse and check for FA, AA, mouse-id */
	if (is_mouse_available()) {
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
	}
	return retval;
}

static uint8_t ps2_keyboard_init(void)
{
	uint8_t retval = 0;
	if (is_keyboard_available()) {
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
	}
	return retval;
}
#endif

/**
 * main action
 */
int main(void)
{
//	wdt_init();

	/* varibles used for the communication and ... */
	uint8_t mouse_detected = 0, keyboard_detected = 0;
	int8_t stat;
	char cmdstr[20];
	data_container_t dc;
	dc.data = rcvbuf;

	/* ... for button functions */
	uint16_t timeout = 0, rpp = 0, rcp = 0;
	uint8_t lock_reset = FALSE, prog_flash = FALSE;

	_delay_ms(2000);

	port_init();
#ifdef PS2
	uart_init(PS2_BAUDRATE);
#else
	uart_init(SYSBOOT_BAUDRATE);
#endif
	uart_set_timeout(200); /* set to 200ms */

	sei(); /* enable irpts */
//UMA	stdout = &mystdout;

#if SUSKA_III_B
	DEBUG_LED_DDR = 1; /* output */
	DEBUG_LED = 1;
  _delay_ms(1000);
  DEBUG_LED = 0;
  _delay_ms(2000);
#endif

#ifdef UMA
  DEBUG_LED_DDR = 1; /* output */
  int p=0;
  for(int i=0;i<5;i++)
  {
    p=IN_RESET_POWER;
    DEBUG_LED = 1;
    _delay_ms(1000);
    DEBUG_LED = 0;
    _delay_ms(1000);
    if(IN_RESET_POWER && p)
    {
       for(int i=0;i<20;i++)
       {
	       DEBUG_LED = 1;
	       _delay_ms(100);
	       DEBUG_LED = 0;
	       _delay_ms(100);
       }
       break;
    }
  }
  if (p && switch_power(TRUE)) 
  { /* power was off */
	  _delay_ms(50); /* wait until stable condition */
	  set_reset_core_pin(TRUE);
	  _delay_ms(5);
	  set_reset_core_pin(FALSE);
	  _delay_ms(500); // was 500ms
	  set_reset_pin(TRUE);
	  _delay_ms(50);
	  set_reset_pin(FALSE);
	  //Start of bootloader
	  _delay_ms(1000);
	  set_reset_pin(TRUE);
	  _delay_ms(1000);
	  set_reset_core_pin(TRUE);
	  _delay_ms(1000);
	  set_reset_core_pin(FALSE);
	  _delay_ms(1000);
	  set_reset_pin(FALSE);
	  _delay_ms(1000);
	  set_reset_pin(TRUE);
	  _delay_ms(1000);
	  set_reset_pin(FALSE);

	  DEBUG_LED = IN_BOOT_REQ;
//	  fstate = FLASH_WAIT_FOR_ADDR_REQ;

  }
#endif

	while(1) {
		if (power_on && IN_BOOT_REQ || fstate == FALSH_WAIT_DATA)
		{
			if (!prog_flash) {
				prog_flash = TRUE;
//				uart_init(SYSBOOT_BAUDRATE); /* change baudrate */
			}
			stat = recv_and_check_dc(&dc, PKG_SIZE);
//if (fstate == FLASH_WRITE_SECTION)
//DEBUG_LED = 1;
			if (stat == 0) {
				if (dc.type == TYPE_REQ) {
					cmdstr[0] = '\0';

					switch (dc.cmd) {
						/* application control commands */
						case CMD_CTL: 
							execute_cmd(&dc, cmdstr); 
							break;

						/* we are ready to start the the FLASH programming */
						case CMD_WRITE: 
							fstate = FLASH_WAIT_FOR_ADDR_REQ;
#ifdef SUSKA_III_B
							spi_enable(TRUE);
#endif
							/* Address has been set through applicationn 
							 * control commands */
							flash_set_addr(flash_start_addr);
							flash_set_mode(); /* erase flash or not */
							break;

						/* receiving content of the FLASH from serial line and
						 * program it. */
						case CMD_DATA:
						case CMD_DATA_LAST:
							fstate = FLASH_WRITE_SECTION;
							/* last package waits 2000ms to create a timeout for the FPGA */
//if (dc.cmd == CMD_DATA_LAST)
//DEBUG_LED = 1;

							flash_program(dc.data, dc.size, (dc.cmd == CMD_DATA_LAST));
							if (dc.cmd == CMD_DATA_LAST) {
								prog_flash = FALSE;
#ifdef SUSKA_III_B
								spi_enable(FALSE);
#endif
							}
							break;
					}
					if (strlen(cmdstr)>0) {
						create_and_send_dc(dc.cmd, TYPE_ACK, strlen(cmdstr), (uint8_t *)cmdstr);
					} else {
						if(dc.cmd==CMD_SYNC)
						{
#ifdef UMA
DEBUG_LED = 0;
#endif
						/* sync gets answered automatically */
						create_and_send_dc(dc.cmd, TYPE_ACK, 0, NULL);
						}
					}
					if (!prog_flash) {
						_delay_ms(5000);
						switch_power(FALSE);
						soft_reset();			
					}
				}
			} else {
				/* handle errors if necessary */
			}
		}

		/* button functions here! */
		timeout=0; lock_reset=FALSE;

		/* check if we press the reset_power or reset_core button */
		do {
			rpp = IN_RESET_POWER;
			rcp = IN_RESET_CORE;
			if (rpp || rcp) {
				_delay_ms(50); /* debounce */

				/* reset power pin has been pressed */
				if (rpp) {
					if (!timeout && switch_power(TRUE)) { /* power was off */
						_delay_ms(50); /* wait until stable condition */
						set_reset_core_pin(TRUE);
						_delay_ms(5);
						set_reset_core_pin(FALSE);
						_delay_ms(500); // was 500ms
						set_reset_pin(TRUE);
						_delay_ms(50);
						set_reset_pin(FALSE);
						lock_reset = TRUE;

#ifdef PS2
						uart_init(PS2_BAUDRATE);
						ps2_init();
						_delay_ms(200);

						if (!mouse_detected) {
							mouse_detected += ps2_mouse_init();
						}
						if (!keyboard_detected) {
							keyboard_detected += ps2_keyboard_init();
						}
#endif
					}
					/* after power/reset and core/rest another 
					   short power reset is necessary or soft reset/warm boot */
					else if (!lock_reset) {
						set_reset_pin(TRUE);
					}

					/* power off after 2 sec (50ms * 40)*/
					if (timeout >= 40) {
						switch_power(FALSE);
					}
					timeout++;
				} else {
					set_reset_pin(FALSE);
				} /* endif reset power */

				/* reset core */
				if (rcp) {
					set_reset_core_pin(TRUE);
					fstate = FLASH_WAIT_FOR_ADDR_REQ;
				} else {
					set_reset_core_pin(FALSE);
				}
			}
		} while (rcp || rpp);
		set_reset_core_pin(FALSE);
		set_reset_pin(FALSE);

		/* PS/2 handling here */
#ifdef PS2
		if (mouse_detected > 0) {
			mouse_poll();
		}
		if (keyboard_detected > 0) {
			keyboard_poll();
		}
		joystick_poll();
#endif
	} /* endless */
}

