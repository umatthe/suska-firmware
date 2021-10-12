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
 * 	08.01.08 add uart_set_timeout to override default timeouts -jc
 *
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>

#include "../../sysboot/shared/uart.h"
#include "fifo.h"

//#define TEST_UART
//#define PARITY EVEN /* NONE, EVEN, ODD */

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#define UMA
#ifdef UMA
#include "sbit.h"
#define DEBUG_LED_DDR           SBIT(DDRC,  6)
#define DEBUG_LED               SBIT(PORTC, 6)
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

static uint16_t uart_timeout;
#define UART_TIMEOUT 2000 /* 2000 x 1ms */

/* Add characters form serial input to the FIFO */
//SIGNAL (SIG_UART_RECV)
#if defined(__AVR_ATmega644__)
ISR(USART0_RX_vect)
{
	if (_inline_fifo_put(&infifo, UDR0)<0)
		overrun = 1;
}
#else
ISR(USART_RXC_vect)
{
	if (_inline_fifo_put(&infifo, UDR)<0)
		overrun = 1;
#ifdef UMA
//	DEBUG_LED=1;
#endif
}
#endif

/**
 * Get one character from outupt FIFO and send it. 
 * After the character has been sent, a new SIG_UART_DATA-IRQ will be triggered 
 * An empty FIFO will deactivate the IRQ.
 */
//SIGNAL (SIG_UART_DATA)
#if defined(__AVR_ATmega644__)
ISR(USART0_UDRE_vect)
{
	if (outfifo.count > 0)
		UDR0 = _inline_fifo_get(&outfifo);
	else
		UCSR0B &= ~(1 << UDRIE0); /* deactivate ISR */
}
#else
ISR(USART_UDRE_vect)
{
	if (outfifo.count > 0)
		UDR = _inline_fifo_get(&outfifo);
	else
		UCSRB &= ~(1 << UDRIE); /* deactivate ISR */
}
#endif

/* wait till all characters have been sent */
static inline void uart_flush(void)
{
#if defined(__AVR_ATmega644__)
	while (UCSR0B & (1 << UDRIE0));
#else
	while (UCSRB & (1 << UDRIE));
#endif
}

/**
 * Set UART timeout
 */
void uart_set_timeout(uint16_t timeout)
{
	uart_timeout = timeout;
}

/**
 * Initialize UART
 */
void uart_init(uint16_t baudrate)
{
	uint8_t sreg = SREG;
	uint16_t ubrr = (uint16_t) (((uint32_t) F_CPU / (16UL * baudrate)) - 1);

#if defined(__AVR_ATmega644__)
	UBRR0H = (uint8_t) (ubrr>>8);
	UBRR0L = (uint8_t) (ubrr);
#else
	UBRRH = (uint8_t) (ubrr>>8);
	UBRRL = (uint8_t) (ubrr);
#endif

	/* deactivate IRQs (for a short time) */
	cli();

	/* turn on UART receiver and transmitter, activate receive-irpt 
	   data mode 8N1, asynchron */
#if defined(__AVR_ATmega644__)
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
#else
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
#endif

#if defined(__AVR_ATmega644__)
#else
	#if (PARITY == EVEN)
		UCSRC |= (1 << UPM1);
	#elif (PARITY == ODD)
		UCSRC |= (1 << UPM0);
	#endif
#endif
   // Set frame format: 8data, 1stop bit, even Parity
	//     UCSR0C = (0<<UMSEL0)|(1<<UCSZ01)|(1<<UCSZ00) |(1<<UPM01);
	//

	/* flush receive buffer (remove possible not valid characters) */
#if defined(__AVR_ATmega644__)
	do {
		/* read UDR (discard characters) */
		UDR0;
	} while (UCSR0A & (1 << RXC0));

	/* reset receive and transmit complete-flags */
	UCSR0A = (1 << RXC0) | (1 << TXC0);
#else
	do {
		/* read UDR (discard characters) */
		UDR;
	} while (UCSRA & (1 << RXC));

	/* reset receive and transmit complete-flags */
	UCSRA = (1 << RXC) | (1 << TXC);
#endif

	/* enable global Irpt-Flag */
	SREG = sreg;

	/* initialize FIFOs for i/o */
	fifo_init(&infifo, inbuf, BUFSIZE_IN);
	fifo_init(&outfifo, outbuf, BUFSIZE_OUT);

	/* set default uart_timeout */
	uart_timeout = UART_TIMEOUT;
}

/**
 * Send one character over serial uart using a stream
 * printf will use this!
 */
static int uart_putchar(char c, FILE *stream)
{
	if (c == '\n')
		uart_putchar('\r', stream);
#if defined(__AVR_ATmega644__)
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
#else
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
#endif
	return 0;
}

/**
 * Send one character over serial uart
 * use a fifo to enhance sending capabilities
 */
int8_t uart_putc(const uint8_t c)
{
	uint8_t ret = fifo_put(&outfifo, c);
#if defined(__AVR_ATmega644__)
	UCSR0B |= (1 << UDRIE0);
#else
	UCSRB |= (1 << UDRIE);
#endif
	return ret;
}

/**
 * Send num of size character over serial uart
 * use a fifo to enhance sending capabilities
 */
int8_t uart_send(const uint8_t *data, uint16_t size)
{
	uint16_t timeout = uart_timeout;

	while (size>0 && timeout>0) {
		if (uart_putc(*data)<0) {
			_delay_ms(1);
			timeout--;
		} else {
			size--;
			timeout = uart_timeout;
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
	uint16_t timeout = uart_timeout, c;

	if (size<=0) return -1;
	do {
		if (fifo_get_nowait(&infifo, &c)<0) {
			_delay_ms(1);
			timeout--;
		} else {
			size--;
			timeout = uart_timeout;
			*data++ = c;
		}
	} while (size>0 && timeout>0);
	return (size) ? -1 : 0;
}

int8_t uart_receive_no_wait(uint8_t *data, uint16_t size)
{
#define RETRIES 2
	uint16_t retry = RETRIES, c;

	if (size<=0) return -1;
	do {
		if (fifo_get_nowait(&infifo, &c)<0) {
			_delay_ms(1);
			retry--;
		} else {
			size--;
			retry = RETRIES;
			*data++ = c;
		}
	} while (size>0 && retry>0);
	return (size) ? -1 : 0;
}


void uart_stop(void)
{
#if defined(__AVR_ATmega644__)
	UCSR0B = 0;
#else
	UCSRB = 0;
#endif
}

#ifdef TEST_UART

#define PAD_LED 3
#define PORT_LED PORTD
#define DDR_LED DDRD

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
