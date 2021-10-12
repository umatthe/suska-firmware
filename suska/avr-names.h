#ifndef _AVR_NAMES_H
#define _AVR_NAMES_H

// F("String"); //put String in Flash
#define F(X) (const __flash char[]) { X }

#include <avr/version.h>

/* check for avr-libc version */
#if __AVR_LIBC_VERSION__ < 10404UL
#error "newer libc version (>= 1.4.4) needed!"
#endif

/* cpu specific configuration registers */
#if defined(__AVR_ATmega32__) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega103__)
#define _ATMEGA32

#define _TIMSK_TIMER1 TIMSK1
#define _UDRIE_UART0 UDRIE
#define _TXEN_UART0 TXEN
#define _RXEN_UART0 RXEN
#define _RXCIE_UART0 RXCIE
#define _UBRRH_UART0 UBRRH
#define _UBRRL_UART0 UBRRL
#define _UCSRA_UART0 UCSRA
#define _UCSRB_UART0 UCSRB
#define _UCSRC_UART0 UCSRC
#define _UCSZ0_UART0 UCSZ00
#define _UCSZ1_UART0 UCSZ01
#define _SIG_UART_RECV_UART0 SIG_USART_RECV
#define _SIG_UART_DATA_UART0 SIG_USART_DATA
#define _UDR_UART0 UDR
#define _UDRE_UART0 UDRE
#define _RXC_UART0 RXC
#define _IVREG MCUCR
#define _SPCR0 SPCR
#define _SPE0 SPE
#define _MCUSR MCUCSR
#define _MSTR0 MSTR
#define _SPSR0 SPSR
#define _SPIF0 SPIF
#define _SPDR0 SPDR
#define _SPI2X0 SPI2X
#define _SPR1 SPR1
#define _SPR0 SPR0
#define _TIFR_TIMER1 TIFR

#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) ||  defined(__AVR_ATmega649A__) ||  defined(__AVR_ATmega649__)
#define _ATMEGA644

#define _TIMSK_TIMER1 TIMSK1
#define _UDRIE_UART0 UDRIE0
#define _TXEN_UART0 TXEN0
#define _RXEN_UART0 RXEN0
#define _RXCIE_UART0 RXCIE0
#define _UBRRH_UART0 UBRR0H
#define _UBRRL_UART0 UBRR0L
#define _UCSRA_UART0 UCSR0A
#define _UCSRB_UART0 UCSR0B
#define _UCSRC_UART0 UCSR0C
#define _UCSZ0_UART0 UCSZ00
#define _UCSZ1_UART0 UCSZ01
#if defined(__AVR_ATmega168__)  || defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
//ASC #define _SIG_UART_RECV_UART0 USART0_RX_vect
//ASC #define _SIG_UART_DATA_UART0 USART0_UDRE_vect
#define _SIG_UART_RECV_UART0 USART_RX_vect
#define _SIG_UART_DATA_UART0 USART_UDRE_vect
#else
//UMA #define _SIG_UART_RECV_UART0 SIG_USART_RECV
//UMA #define _SIG_UART_DATA_UART0 SIG_USART_DATA
#define _SIG_UART_RECV_UART0 USART0_RX_vect
#define _SIG_UART_DATA_UART0 USART0_UDRE_vect
#endif
#define _UDR_UART0 UDR0
#define _UDRE_UART0 UDRE0
#define _RXC_UART0 RXC0
#define _TXC_UART0 TXC0
#define _IVREG MCUCR
#if defined(__AVR_ATmega644__)
#define _SPCR0 SPCR0
#define _SPE0 SPE0
#define _MSTR0 MSTR0
#define _SPSR0 SPSR0
#define _SPIF0 SPIF0
#define _SPDR0 SPDR0
#define _SPI2X0 SPI2X0
#define _SPR1 SPR1
#define _SPR0 SPR0
#else
#define _SPCR0 SPCR
#define _SPE0 SPE
#define _MSTR0 MSTR
#define _SPSR0 SPSR
#define _SPIF0 SPIF
#define _SPDR0 SPDR
#define _SPI2X0 SPI2X
#define _SPR1 SPR1
#define _SPR0 SPR0
#endif
#define _TIFR_TIMER1 TIFR1

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || defined(__AVR_ATmega32U4__)
#define _ATMEGA644P

#define _TIMSK_TIMER1 TIMSK1
#define _UDRIE_UART0 UDRIE0
#define _TXEN_UART0 TXEN0
#define _RXEN_UART0 RXEN0
#define _RXCIE_UART0 RXCIE0
#define _UBRRH_UART0 UBRR0H
#define _UBRRL_UART0 UBRR0L
#define _UCSRA_UART0 UCSR0A
#define _UCSRB_UART0 UCSR0B
#define _UCSRC_UART0 UCSR0C
#define _UCSZ0_UART0 UCSZ00
#define _UCSZ1_UART0 UCSZ01
//#define _SIG_UART_RECV_UART0 SIG_USART0_RECV
//#define _SIG_UART_DATA_UART0 SIG_USART0_DATA
#define _SIG_UART_RECV_UART0 USART0_RX_vect
#define _SIG_UART_DATA_UART0 USART0_UDRE_vect
#define _UDR_UART0 UDR0
#define _UDRE_UART0 UDRE0
#define _RXC_UART0 RXC0
#define _TXC_UART0 TXC0

#define _UDRIE_UART1 UDRIE1
#define _TXEN_UART1 TXEN1
#define _RXEN_UART1 RXEN1
#define _RXCIE_UART1 RXCIE1
#define _UBRRH_UART1 UBRR1H
#define _UBRRL_UART1 UBRR1L
#define _UCSRA_UART1 UCSR1A
#define _UCSRB_UART1 UCSR1B
#define _UCSRC_UART1 UCSR1C
#define _UCSZ0_UART1 UCSZ10
#define _UCSZ1_UART1 UCSZ11
#define _UDR_UART1 UDR1
#define _UDRE_UART1 UDRE1
#define _RXC_UART1 RXC1
#define _TXC_UART1 TXC1
//#define _SIG_UART_RECV_UART1 SIG_USART1_RECV
//#define _SIG_UART_DATA_UART1 SIG_USART1_DATA
#define _SIG_UART_RECV_UART1 USART1_RX_vect
#define _SIG_UART_DATA_UART1 USART1_UDRE_vect

#define _IVREG MCUCR
#define _SPCR0 SPCR
#define _SPE0 SPE
#define _MSTR0 MSTR
#define _SPSR0 SPSR
#define _SPIF0 SPIF
#define _SPDR0 SPDR
#define _SPI2X0 SPI2X
#define _SPR1 SPR1
#define _SPR0 SPR0
#define _TIFR_TIMER1 TIFR1
#else
#error "this cpu isn't supported yet!"
#endif

#ifndef TWBR
#define TWBR TWBR0
#define TWCR TWCR0
#define TWDR TWDR0
#define TWSR TWSR0
#endif

#endif /* _AVR_NAMES_H */
