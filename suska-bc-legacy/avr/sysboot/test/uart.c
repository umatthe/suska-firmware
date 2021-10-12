/***
 * Program to receive and transmit a stream of characters over RS232/USB
 * The program secures the data transmission with REQ, ACK and a CRC 
 *
 * Copyright (c) 2007, Inventronik GmbH
 * All rights reserved.
 *
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
 *
 * Added:
 * 	25.11.07 parity handling -jc
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "uart.h"
#include "fifo.h"

//#define TEST_UART
#define BAUDRATE 38400
//#define PARITY EVEN /* NONE, EVEN, ODD */

#define PAD_LED 3
#define PORT_LED PORTD
#define DDR_LED DDRD

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

static int uart_putchar(char c, FILE *stream);
FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

/* FIFO for i/o (each 40 characters) */
#define BUFSIZE_IN	0x80
uint8_t inbuf[BUFSIZE_IN];
fifo_t infifo;

#define BUFSIZE_OUT 0x80
uint8_t outbuf[BUFSIZE_OUT];
fifo_t outfifo;
int8_t overrun = 0; /* 1-indicates an overrun error */

#define UART_TIMEOUT 2000 /* 2000 x 1ms */

/* Add characters form serial input to the FIFO */
SIGNAL (SIG_UART_RECV)
{
	if (_inline_fifo_put(&infifo, UDR)<0)
		overrun = 1;
}

/**
 * Get one character from outupt FIFO and send it. 
 * After the character has been sent, a new SIG_UART_DATA-IRQ will be triggered 
 * An empty FIFO will deactivate the IRQ.
 */
SIGNAL (SIG_UART_DATA)
{
	if (outfifo.count > 0)
		UDR = _inline_fifo_get(&outfifo);
	else
		UCSRB &= ~(1 << UDRIE); /* deactivate ISR */
}

/* wait till all characters have been sent */
static inline void uart_flush(void)
{
	while (UCSRB & (1 << UDRIE));
}

void uart_init(void)
{
	uint8_t sreg = SREG;
	uint16_t ubrr = (uint16_t) ((uint32_t) F_CPU/(16*BAUDRATE) - 1);

	UBRRH = (uint8_t) (ubrr>>8);
	UBRRL = (uint8_t) (ubrr);

	/* deactivate IRQs (for a short time) */
	cli();

	/* turn on UART receiver and transmitter, activate receive-irpt 
	   data mode 8N1, asynchron */
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);

	#if (PARITY==EVEN)
		UCSRC |= (1 << UPM1);
	#elif (PARITY=ODD)
		UCSRC |= (1 << UPM0);
	#endif

    // Set frame format: 8data, 1stop bit, even Parity
	//     UCSR0C = (0<<UMSEL0)|(1<<UCSZ01)|(1<<UCSZ00) |(1<<UPM01);
	//

	/* flush receive buffer (remove possible not valid characters) */
	do {
		/* read UDR (discard characters) */
		UDR;
	} while (UCSRA & (1 << RXC));

	/* reset receive and transmit complete-flags */
	UCSRA = (1 << RXC) | (1 << TXC);

	/* enable global Irpt-Flag */
	SREG = sreg;

	/* initialize FIFOs for i/o */
	fifo_init(&infifo, inbuf, BUFSIZE_IN);
	fifo_init(&outfifo, outbuf, BUFSIZE_OUT);
}

/**
 * Send one character over serial uart using a stream
 * printf will use this!
 */
static int uart_putchar(char c, FILE *stream)
{
	if (c == '\n')
		uart_putchar('\r', stream);
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
	return 0;
}

/**
 * Send one character over serial uart
 * use a fifo to enhance sending capabilities
 */
static int8_t uart_putc(const uint8_t c)
{
	uint8_t ret = fifo_put(&outfifo, c);
	UCSRB |= (1 << UDRIE);
	return ret;
}

/**
 * Send num of size character over serial uart
 * use a fifo to enhance sending capabilities
 */
int8_t uart_send(const uint8_t *data, uint16_t size)
{
	uint16_t timeout = UART_TIMEOUT;

	while (size>0 && timeout>0) {
		if (uart_putc(*data)<0) {
			_delay_ms(1);
			timeout--;
		} else {
			size--;
			timeout = UART_TIMEOUT;
			data++;
		}
	}
	return (size) ? -1 : 0;
}

/**
 * Receive num of size character from serial uart
 * use a fifo to enhance sending capabilities
 */
int8_t uart_receive(uint8_t *data, uint16_t size)
{
	uint16_t timeout = UART_TIMEOUT, c;

	do {
		if (fifo_get_nowait(&infifo, &c)<0) {
			_delay_ms(1);
			timeout--;
		} else {
			size--;
			timeout = UART_TIMEOUT;
			*data++ = c;
		}
	} while (size>0 && timeout>0);
	return (size) ? -1 : 0;
}

#ifdef TEST_UART
int main(void)
{
	char c;

	uart_init();
	enable_interrupts();

	stdout = &mystdout;

	/* LED-Port auf OUT */
	DDR_LED  |= (1 << PAD_LED);

	for(;;) {
		/* LED an */
		PORT_LED |= (1 << PAD_LED);

		delay_ms(5000);
		printf("My num was: %d\n", 1234);

		do {
			c = uart_getc_nowait();
			if(c!=0xFF)
				printf("-> '%c' (%d)\n", c, c);
		} while(c!=0xFF);

		/* LED aus */
		PORT_LED &= ~(1 << PAD_LED);
		delay_ms(500);
	}
}
#endif
