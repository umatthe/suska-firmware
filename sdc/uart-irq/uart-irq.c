#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../fifo/fifo.h"
#ifndef _UART_IRQ_NOTIMEOUT_
#include "../timer/tick.h"
#endif
#ifdef SHELL_POLL
#include "../shell/shell.h"
#endif

#include "uart-irq.h"

#ifdef USE_PRINTF
#include <stdio.h>
static int uart_putchar(char c, FILE *stream);
FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
#endif

#ifdef UART_USE_SUSKA
static uint16_t uart_timeout;
#define UART_TIMEOUT 2000 /* 2000 x 1ms */
#endif

// FIFO-Objekte und Puffer für die Ein- und Ausgabe 

uint8_t inbuf[UART_BUFSIZE_IN];
fifo_t infifo;

//#define BUFSIZE_OUT 0x40
//uint8_t outbuf[BUFSIZE_OUT];
//fifo_t outfifo;

void uart_init(void)
{
 
   uart_setbaud(UART_UBRR);

    /* set mode */
    /* 8N1, asynchronous */
    _UCSRC_UART0 = UART_UCSRC;

    /* enable transmitter and receiver */
    _UCSRB_UART0 = _BV(_TXEN_UART0) | _BV(_RXEN_UART0); 

#ifdef UART_USE_SUSKA
    uart_timeout=UART_TIMEOUT;
#endif
}

void uart_setbaud(uint16_t ubrr)
{

    /* set baud rate */
    _UBRRH_UART0 = (uint8_t)(ubrr >> 8);  /* high byte */
    _UBRRL_UART0 = (uint8_t)ubrr;         /* low byte */

}

void uart_irqinit(void)
{
    uint8_t sreg = SREG;
    cli();

    /* enable receive IRQ */
    _UCSRB_UART0 |= _BV(_RXCIE_UART0);

    // Flush Receive-Buffer (entfernen evtl. vorhandener ungültiger Werte) 
    do
    {
        // UDR auslesen (Wert wird nicht verwendet) 
        _UDR_UART0;
    } while (_UCSRA_UART0 & (1 << _RXC_UART0));

    // Rücksetzen von Receive und Transmit Complete-Flags 
    _UCSRA_UART0 = _BV(_RXC_UART0) | _BV(_TXC_UART0);


    // Global Interrupt-Flag wieder herstellen 
    SREG = sreg;

    // FIFOs für Ein- und Ausgabe initialisieren 
    fifo_init (&infifo,   inbuf, UART_BUFSIZE_IN);
//    fifo_init (&outfifo, outbuf, BUFSIZE_OUT);
    sei();
}

// Empfangene Zeichen werden in die Eingangs-FIFO gespeichert und warten dort 
SIGNAL (_SIG_UART_RECV_UART0)
{
    _inline_fifo_put (&infifo, _UDR_UART0);
}

// Ein Zeichen aus der Ausgabe-FIFO lesen und ausgeben 
// Ist das Zeichen fertig ausgegeben, wird ein neuer SIG_UART_DATA-IRQ getriggert 
// Ist die FIFO leer, deaktiviert die ISR ihren eigenen IRQ. 
/*
SIGNAL (_SIG_UART_DATA_UART0)
{
    if (outfifo.count > 0)
       _UDR_UART0 = _inline_fifo_get (&outfifo);
    else
       _UCSRB_UART0 &= ~(1 << _UDRIE_UART0);
}

int uart_putc (const uint8_t c)
{
    int ret = fifo_put (&outfifo, c);
	
    _UCSRB_UART0 |= (1 << UDRIE);
	 
    return ret;
}
*/

int uart_getc_nowait ()
{
    return fifo_get_nowait (&infifo);
}

uint8_t uart_getc_wait ()
{
    return fifo_get_wait (&infifo);
}

void noinline uart_putc(uint8_t data)
{

    while (!(_UCSRA_UART0 & _BV(_UDRE_UART0)));

    _UDR_UART0 = data;

}

void noinline uart_putf(void *buffer)
{

    uint8_t c;
    uint8_t *b = (uint8_t *)buffer;

    while ((c = pgm_read_byte(b)) != '\0') {
        uart_putc(c);
        b++;
    }

} 

void noinline uart_puts(char *buffer)
{

    uint8_t *b = (uint8_t *)buffer;

    while (*b != '\0') {
        uart_putc(*b);
        b++;
    }

} 

void noinline uart_puthexnibble(uint8_t data)
{

    if (data > 9) {
        data -= 10;
        uart_putc('A' + data);
    } else {
        uart_putc('0' + data);
    }

} 

void noinline uart_puthexbyte(uint8_t data)
{

    /* first high, then low nibble */
    uart_puthexnibble((data & 0xF0) >> 4);
    uart_puthexnibble(data & 0x0F);

}

void noinline uart_puthexword(uint16_t data)
{
	uart_puthexbyte((data & 0xFF00) >> 8);
	uart_puthexbyte(data & 0x00FF);
}

void noinline uart_puthexlong(uint32_t data)
{
	uart_puthexword((data & 0xFFFF0000) >> 16);
	uart_puthexword(data & 0x0000FFFF);
}

void noinline uart_putdecbyte(uint8_t data)
{

    uint8_t pot10[] = {100, 10};

    for (uint8_t i=0; i<2; i++) {

        uint16_t p = pot10[i];
        uint8_t digit = 48;

        while (data >= p) {
            digit++;
            data -= p;
        }

        uart_putc(digit);

    }

    uart_putc(data+48);

}

void noinline uart_eol(void)
{
    uart_puts_P("\r\n");
} 

#ifdef USE_PRINTF
/**
 *  Send one character over serial uart using a stream
 *  printf will use this!
 **/
static int uart_putchar(char c, FILE *stream)
{
	if (c == '\n') uart_putchar('\r', stream);
	uart_putc(c);

	return 0;
}
#endif

uint8_t noinline uart_getline_nowait(uint8_t *line, uint8_t maxlen)
{
	uint8_t len=0;
	int 	ch;

	do 
	{
		ch=uart_getc_nowait();

		if (ch == -1) //nothing in input buffer
		{
			break;
		}
		else
		{
			line[len]=ch;
		}

		if(line[len]==0xd)
		{
#ifdef _UART_IRQ_EOL_D_OR_A_
			break;
#else
			continue;
#endif
		}
		else if(line[len]==0xa)
		{
			break;
		}
		else if(line[len]==0x8) // Backspace
		{
			if(len>0) 
			{
				line[len]=0;
				len--;
			}
			line[len]=0;
			continue;
		}
		else
		{
			len++;
		}

	} while(len<maxlen);

	line[len]=0;

	return len;
}

uint8_t noinline uart_getline(uint8_t *line, uint8_t maxlen)
{
	uint8_t len=0;
	int 	ch;

#ifndef _UART_IRQ_NOTIMEOUT_
	uint32_t start=tick;
#endif
	do 
	{
#ifdef SHELL_POLL
		SHELL_POLL();
#endif

#ifndef _UART_IRQ_NOTIMEOUT_
		// Timeout
		if((tick-start) > 300)
		{
			len=0;
			break;
		}
#endif
		ch=uart_getc_nowait();

		if (ch == -1) //nothing in input buffer
		{
#ifdef _UART_IRQ_NOWAIT_EMPTY_
			if(len==0) break;
			else continue;
#else
			continue;
#endif
		}
		else
		{
			line[len]=ch;
		}

		if(line[len]==0xd)
		{
#ifdef _UART_IRQ_EOL_D_OR_A_
			break;
#else
			continue;
#endif
		}
		else if(line[len]==0xa)
		{
			break;
		}
		else if(line[len]==0x8) // Backspace
		{
			if(len>0) 
			{
				line[len]=0;
				len--;
			}
			line[len]=0;
			continue;
		}
		else
		{
			len++;
		}
	} while(len<maxlen);

	line[len]=0;

#ifdef _UART_IRQ_ONLY_RETURN_
	if((ch == 0x0d) && (len==0))
	{
		line[0]=0xd;
		line[1]=0;
	}
#endif
	return len;
}


#ifdef UART_USE_SUSKA
/**
 * Send num of size character over serial uart
 * use a fifo to enhance sending capabilities
 */
int8_t uart_send(const uint8_t *data, uint16_t size)
{
#if 0
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
#else
        while (size>0)
        {
                uart_putc(*data);
		size--;
        }
#endif
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
                if ((c=fifo_get_nowait(&infifo))<0) {
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
#endif
