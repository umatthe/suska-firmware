#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include "uart.h"
#include <util/delay.h> // Reihenfolge beachten wg. F_CPU

#undef putchar

/* USART initialisieren */
void uart_init(void)
{
	/* Baudrate einstellen ( Doppelter Modus ) */
	UBRRH = (unsigned char) (UBRR_BAUD >> 8);
	UBRRL = (unsigned char) UBRR_BAUD;

	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);

	/* Einstellen des Datenformats: 8 Datenbits, 1 Stoppbit */
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);

	sei();
}

void uputch(char x)
{
	/* send register empty? */
	loop_until_bit_is_set(UCSRA, UDRE);

	/* send it */
	UDR = x;
}

void uputs(char *s)
{
	int n = 0;

	while(*(s)) uputch(*s++), n++;
}

void uputu(unsigned long d)
{
	if (d >= 10) uputu(d / 10);
	uputch(d % 10 + '0');
}

void uputd(long d)
{
	if (d < 0) {
		uputch('-'); d = -d;
	}
	uputu(d);
}

void uputx(unsigned long d)
{
	if (d >= 0x10) uputx(d / 0x10);
	uputch(d % 0x10 > 9 ? d % 16 + 'A' - 10 : d % 16 + '0');
}

volatile unsigned char buffer[RING_SIZE];
volatile unsigned short irr = 0;
volatile unsigned short irw = 0;
volatile char count = 0;

ISR(USART_RXC_vect)
{
	static unsigned char zeichen = 0;

	zeichen = UDR;
	buffer[irw] = zeichen;
	irw += 1; if (irw == RING_SIZE) irw = 0;
	count++;
}

short int ugetchar(void)
{
	unsigned char c;

	if (count > 0) {
		cli();
		c = buffer[irr];
		irr++; 
		if (irr == RING_SIZE) 
			irr = 0;
		count--;
		sei();
		return c;
	} else return -1;
}

void uputsnl(char *s )
{
	uputs( s );
	uputs( "\n" );
}

int putchar(int c)
{
	uputch(c);
	return 1;
}
