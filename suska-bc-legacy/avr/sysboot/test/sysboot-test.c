/**
 * FPGA-Bootloader for programming the suska Flash-Proms
 *
 * this firmware also contains the functions needed to
 * use the 4 buttons of the suska board to create resets 
 * and the power on/off signals.
 *
 * Wish to have:
 * Power management
 * Battery-Voltage controller PIN PA6 <10,6V -> high PB7
 * PA7 (3,3V) Reference
 *
 * Important: JTAG fuse must be programmed (hfuse: 0xDF)
 */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "uart.h"
#include "sbit.h"

#define FALSE	0
#define TRUE	1


/* Input port pins */
#define IN_RESET_POWER		SBIT(PINB,  2)	/* Port B pin 2, active high */
#define IN_RESET_CORE		SBIT(PINA,  5)	/* Port A pin 5, active high */
#define IN_BOOT_REQ			SBIT(PIND,  3)	/* Port D pin 3, active high */
#define SPI_MISO			SBIT(PINB,  6)	/* Port B pin 6, Master in */
#define FPGA_CONFIG_DONE	SBIT(PINB,  0)	/* Port B pin 0, FPGA configuration done, active high */

/* Output port pins */
#define OUT_RESET			SBIT(PORTC, 3)	/* Port C pin 3, active high */
#define OUT_RESET_DDR		SBIT(DDRC,  3)
#define OUT_RESET_CORE		SBIT(PORTC, 2)	/* Port C pin 2, active high */
#define OUT_RESET_CORE_DDR	SBIT(DDRC,  2)

#define OUT_PS_50			SBIT(PORTD, 7)	/* Port D pin 7, active high */
#define OUT_PS_50_DDR		SBIT(DDRD,  7)
#define OUT_PS_33			SBIT(PORTD, 6)	/* Port D pin 6, active high */
#define OUT_PS_33_DDR		SBIT(DDRD,  6)
#define OUT_PS_12			SBIT(PORTD, 5)	/* Port D pin 5, active high */
#define OUT_PS_12_DDR		SBIT(DDRD,  5)
#define OUT_PS_VBIAS		SBIT(PORTD, 4)	/* Port D pin 4, active high */
#define OUT_PS_VBIAS_DDR	SBIT(DDRD,  4)

#define OUT_BOOT_ACK		SBIT(PORTC, 6)	/* Port C pin 6, active high */
#define OUT_BOOT_ACK_DDR	SBIT(DDRC,  6)

#define SPI_MOSI			SBIT(PORTB, 5)	/* Port B pin 5, Master out */
#define SPI_MOSI_DDR		SBIT(DDRB,  5)
#define SPI_CLK				SBIT(PORTB, 7)	/* Port B pin 7, SPI clock output */
#define SPI_CLK_DDR			SBIT(DDRB,  7)
#define SPI_SS				SBIT(PORTB, 4)	/* Port B pin 4, SPI clock output */
#define SPI_SS_DDR			SBIT(DDRB,  4)

#define MODE_ERASE_WRITE_READ	0x23
#define MODE_WRITE_READ			0x20
#define MODE_READ				0x17
#define MODE_ERASE				0x10

/* !!!!!!!!!!!!!!!!!!!! */
/* Setup used MODE here */
/* !!!!!!!!!!!!!!!!!!!! */
//#define MODE					MODE_ERASE_WRITE_READ
#define MODE					MODE_READ


volatile uint8_t power_on = FALSE;
volatile uint8_t reset_on = FALSE;
volatile uint8_t reset_core_on = FALSE;

enum flash_states {
	FLASH_WAIT_FOR_ADDR_REQ,
	FLASH_SEND_MODE,
	FLASH_WRITE_SECTION,
	FLASH_READ_SECTION,
	FLASH_DONE
};

volatile enum flash_states fstate = FLASH_WAIT_FOR_ADDR_REQ;

/* needed for stdio */
extern FILE mystdout;

/**
 * Init port pins (after reset all port pins are inputs)
 */
void port_init( void)
{
	/* set data direction register and outputs */
	OUT_RESET_CORE = OUT_RESET = 0;
	OUT_RESET_CORE_DDR = OUT_RESET_DDR = 1; /* output */

	OUT_PS_50 = OUT_PS_33 = OUT_PS_12 = OUT_PS_VBIAS = 0;
	OUT_PS_50_DDR = OUT_PS_33_DDR = OUT_PS_12_DDR = OUT_PS_VBIAS_DDR = 1; /* output */

	OUT_BOOT_ACK = 0;
	OUT_BOOT_ACK_DDR = 1; /* output */

    /* Enable SPI, Master, MSB first, clock idle high (CPOL=1 - set), sample on
	   falling edge (CPHA=0 - not set), clock rate fck/16 (see Wikipedia) */
	SPI_CLK_DDR = SPI_MOSI_DDR = SPI_SS_DDR = 1;
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0) | _BV(CPOL); /* _BV(CPHA); */
}

/**
 * SPI master sends to the slave
 */
uint8_t spi_master_send(uint8_t data)
{
    /* start transmission */
	SPDR = data;
		
	/* wait for transmission complete */
	while (!(SPSR & _BV(SPIF)));
	return SPDR;
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
 * SPI master sends a dummy byte while receiving at the
 * same time from the slave
 */
void wait_for_fpga_requests(uint8_t mode)
{
	uint16_t i=0, j=0, k=0;

	if (IN_BOOT_REQ && FPGA_CONFIG_DONE) {
		printf("boot req on\n");
/*
tmp = spi_master_rcv();
if (tmp != 0xFF) printf("read spi 1: 0x%02x\n", tmp);
tmp = spi_master_rcv();
if (tmp != 0xFF) printf("read spi 2: 0x%02x\n", tmp);
*/
		if (fstate==FLASH_WAIT_FOR_ADDR_REQ) {
			printf("send programming start addr to FPGA (0x0000)\n");

			spi_master_send(0x18); /* msb of a 19bit start addr (bit 19..12) */
			spi_master_send(0x0); /* lsb of a 19bit start addr (bit 11..4) */

			_delay_ms(400); /* indicate end of write section */
			signal_ack();

			fstate = FLASH_SEND_MODE;

		} else if (fstate==FLASH_SEND_MODE) {

			printf("send mode to FPGA (currently 0x%02x)\n", mode);

			spi_master_send(mode); /* 0x17, 0x23, 0x20 or 0x10 */
			_delay_ms(400); /* indicate end of write section */
			signal_ack();

			printf("start to erase flash prom!\n");
			fstate = (mode==MODE_READ ? FLASH_READ_SECTION : FLASH_WRITE_SECTION);

		} else if (fstate==FLASH_WRITE_SECTION) {
			printf("send 64 bytes flash data to FPGA\n");
//			for (j=0; j<4096; j++) {
//			for (j=0; j<64; j++) {
//				for (i=0; i<256; i+=2) {
				for (i=0; i<64; i+=2) {
					while (!IN_BOOT_REQ); /* wait for FPGA request */
					printf("send: %02d received: %02d\n", i, spi_master_send((uint8_t) (i)));
					printf("send: %02d received: %02d\n", i+1, spi_master_send((uint8_t) (i+1)));
					signal_ack();
				}
//			}
			fstate = FLASH_READ_SECTION;

			_delay_ms(2000); /* indicate end of write section */
		} else if (fstate==FLASH_READ_SECTION) {
			printf("read back 64 bytes flash prom\n");

			/* read back */
//			for (j=0; j<4096; j++) {
//			for (j=0; j<64; j++) {
//				for (i=0; i<256; i+=2) {
				for (i=0; i<64; i+=2) {
					if (!(i%32)) {
						printf("\n%06lx: ", ((uint32_t)j*256+i));
						k=0;
					}
					else if (k==8) {
						printf("- ");
					}
					k++;

					while (!IN_BOOT_REQ); /* wait for FPGA request */

					{ uint8_t l,m; l=spi_master_rcv(); m=spi_master_rcv();
					printf("%02x %02x ", l, m);}

					signal_ack();
					_delay_us(1);
				}
//			}
			printf("\nend of read block\n");
			fstate = FLASH_DONE;

		} else if (fstate==FLASH_DONE) {
			printf("flash state machine done (end it with a 4 sec timeout)\n");
			_delay_ms(4000); /* indicate end of write section */
		}
	}
}

/**
 * Switch power on/off
 */
uint8_t switch_power(uint8_t on)
{
	uint8_t result = FALSE;

	if (power_on != on) {
		if (on) {
			result = TRUE;
			OUT_PS_50 = OUT_PS_33 = OUT_PS_12 = OUT_PS_VBIAS = 1;
			while (FPGA_CONFIG_DONE)
				;
			printf("switch on\n");
		} else {
			OUT_PS_50 = OUT_PS_33 = OUT_PS_12 = OUT_PS_VBIAS = 0;
			printf("switch off\n");
		}
	}
	power_on = on;
	return result;
}

/**
 * Set reset pin
 */
void set_reset_pin(uint8_t on)
{
	if (reset_on != on) {
		if (on) {
			printf("Reset power on\n");
			OUT_RESET = 1;
		} else {
			printf("Reset power off\n");
			OUT_RESET = 0;
		}
	}
	reset_on = on;
}

/**
 * Set reset core pin
 */
void set_reset_core_pin(uint8_t on)
{
	if (reset_core_on != on) {
		if (on) {
			printf("Reset core on\n");
			OUT_RESET_CORE = 1;
		} else {
			printf("Reset core off\n");
			OUT_RESET_CORE = 0;
		}
	}
	reset_core_on = on;
}

/**
 * main action here
 */
int main(void)
{
	uint16_t t=0, rpp=0, rcp=0;
	uint8_t lock_reset=FALSE, mode;
	
	mode = MODE;

	port_init();
	uart_init();
	stdout = &mystdout;

	printf("-- AVR start --\n");

	while (1) {
		t=0; lock_reset=FALSE;
		/* check if we press the reset/power button for more than 4s */
		do {
			rpp = IN_RESET_POWER;
			rcp = IN_RESET_CORE;
			if (rpp || rcp) {
				printf("Key pressed (IN_RESET_POWER_PIN): %d\n", t);
				_delay_ms(50); /* debounce */

				/* reset power pin has been pressed */
				if (rpp) {
					if (!t && switch_power(TRUE)) { /* power was off */
						_delay_ms(50); /* wait until stable condition */
						set_reset_core_pin(TRUE);
						_delay_ms(5);
						set_reset_core_pin(FALSE);
						_delay_ms(5);
						set_reset_pin(TRUE);
						_delay_ms(5);
						set_reset_pin(FALSE);
						lock_reset=TRUE;

					} else if (!lock_reset) {
						set_reset_pin(TRUE);
						fstate = FLASH_WAIT_FOR_ADDR_REQ;
					}

					/* power off after 4 sec */
					if (t==80) {
						switch_power(FALSE);
					}
					t++;
				} else {
					set_reset_pin(FALSE);
				}

				/* reset core */
				if (rcp) {
					set_reset_core_pin(TRUE);
				} else {
					set_reset_core_pin(FALSE);
				}
			}
		} while (rcp || rpp);
		set_reset_core_pin(FALSE);
		set_reset_pin(FALSE);
		wait_for_fpga_requests(mode);
	}
}

