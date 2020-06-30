#ifndef _UART_IRQ_H_
#define _UART_IRQ_H_

// Konfiguration, um das Verhalten der uart_getline Funktion 
// zu beeinflussen:
//
//#define _UART_IRQ_NOWAIT_EMPTY_
//Normalerweise wartet uart_getline, bis das Zeilenende
//erkannt (oder die Maximallaenge erreicht) wurde.
//Mit diesem Schalter kehrt die Funktion zurück, falls
//nicht wenigstens ein Zeichen gelesen werden kann.
//Sobald ein Zeichen gelesen wurde, wird wieder bis zum Zeilenende gewartet.

//#define _UART_IRQ_EOL_D_OR_A_
//Dieses define bewirkt, dass die ASCII Zeichen 0x0a oder 0x0d
//als Zeilenende erkannt werden. Ohne dieses Define wird als
//Zeilenende die ASCII Sequenz 0x0d 0x0a erwartet. NMEA Saetze
//z.B. Enden mit dieses Sequenz.

//#define _UART_IRQ_NOTIMEOUT_
//Dieses define bewirkt, dass endlos gewartet wird. Default ist
//ist ein timeout nach 300 ticks.

//#define _UART_IRQ_ONLY_RETURN_
//Normalerweise liefert die Funktion Laenge 0 und einen String, der nichts
//enthält, falls die Eingabe nur "Return" enthält. Dieses Define bewirkt,
//dass Zwar Laenge 0 zurueckgegeben wird, Der String aber 0x0d enthält.

#include <avr/pgmspace.h>
#include "config.h"

/* uart defines */
#ifndef UART_BAUDRATE
#define UART_BAUDRATE 115200
#warning "UART_BAUDRATE not defined using default 115200"
#endif

#ifndef UART_BUFSIZE_IN
#define UART_BUFSIZE_IN 100 
#warning "UART_BUFSIZE_IN not defined using default 100"
#endif

#ifndef noinline
#define noinline __attribute__((noinline))
#endif

/* define uart mode (8N1) */
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega32__)
/* in atmega8 and atmega32, we need a special switching bit
 * for addressing UCSRC */
#define UART_UCSRC (_BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1))

#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega649A__) ||  defined(__AVR_ATmega649__)
/* in the new controller generation, this isn't needed any more */
#define UART_UCSRC (_BV(_UCSZ0_UART0) | _BV(_UCSZ1_UART0))
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__)
/* in the new controller generation, this isn't needed any more */
#define UART_UCSRC (_BV(_UCSZ0_UART0) | _BV(_UCSZ1_UART0))

#ifdef USE_UART1
#undef _UDRIE_UART0 
#undef _TXEN_UART0 
#undef _RXEN_UART0 
#undef _RXCIE_UART0 
#undef _UBRRH_UART0 
#undef _UBRRL_UART0 
#undef _UCSRA_UART0 
#undef _UCSRB_UART0 
#undef _UCSRC_UART0 
#undef _UCSZ0_UART0 
#undef _UCSZ1_UART0 
#undef _UDR_UART0 
#undef _UDRE_UART0
#undef _RXC_UART0
#undef _TXC_UART0
#undef _SIG_UART_RECV_UART0 
#undef _SIG_UART_DATA_UART0 

#define _UDRIE_UART0 _UDRIE_UART1
#define _TXEN_UART0 _TXEN_UART1
#define _RXEN_UART0 _RXEN_UART1
#define _RXCIE_UART0 _RXCIE_UART1
#define _UBRRH_UART0 _UBRRH_UART1
#define _UBRRL_UART0 _UBRRL_UART1
#define _UCSRA_UART0 _UCSRA_UART1
#define _UCSRB_UART0 _UCSRB_UART1
#define _UCSRC_UART0 _UCSRC_UART1
#define _UCSZ0_UART0 _UCSZ0_UART1
#define _UCSZ1_UART0 _UCSZ1_UART1
#define _UDR_UART0 _UDR_UART1
#define _UDRE_UART0 _UDRE_UART1
#define _RXC_UART0 _RXC_UART1
#define _TXC_UART0 _TXC_UART1
#define _SIG_UART_RECV_UART0 _SIG_UART_RECV_UART1
#define _SIG_UART_DATA_UART0 _SIG_UART_DATA_UART1 
#elif defined(USE_UART0)
// Definition in ../avr-names.h are ok, no need to redefine.
#else
#error "USE_UART0 or USE_UART1 needs to be defined."
#endif

#elif defined(__AVR_ATmega103__)
#error "Mal guggn"
#else
#error "this cpu isn't supported by uart.h yet!"
#endif

#if (UART_BAUDRATE == 115200 && F_CPU == 16000000)
#   define UART_UBRR 8 /* 16Mhz, 115200 */
#elif (UART_BAUDRATE == 38400 && F_CPU == 16000000)
#   define UART_UBRR 25 /* 16Mhz, 38400 */
#elif (UART_BAUDRATE == 4800 && F_CPU == 16000000)
#   define UART_UBRR 207 /* 16Mhz, 4800 */
#elif (UART_BAUDRATE == 9600 && F_CPU == 16000000)
#   define UART_UBRR 103 /* 16Mhz, 9600 */
#elif (UART_BAUDRATE == 115200 && F_CPU == 20000000)
#   define UART_UBRR 10 /* 20Mhz, 115200 */
#elif (UART_BAUDRATE == 4800 && F_CPU == 1000000)
#   define UART_UBRR 12 /* 1Mhz, 4800 */
#elif (UART_BAUDRATE == 4800 && F_CPU == 8000000)
#   define UART_UBRR 103 /* Mmhz, 4800 */
#elif (UART_BAUDRATE == 9600 && F_CPU == 8000000)
#   define UART_UBRR 51 /* 8Mhz, 9600 */
#elif (UART_BAUDRATE == 38400 && F_CPU == 8000000)
#   define UART_UBRR 12 /* 8Mhz, 38400 */
#elif (UART_BAUDRATE == 38400 && F_CPU == 14745600)
#   define UART_UBRR 23 /* 14.7456Mhz, 38400 */
#else
#   define UART_UBRR (F_CPU/(UART_BAUDRATE * 16L)-1)
#   warning "Default UART_UBRR Calculated"
#endif


/* prototypes */

/* init uart with given baudrate and mode, enable rx and tx */
void uart_setbaud(uint16_t ubrr);
void uart_init(void);
void uart_irqinit(void);

/* output one byte, do busy waiting while transfer is in progress */
void noinline uart_putc(uint8_t data);

/* output data from flash while byte is non-null */
void noinline uart_putf(void *buffer);

/* output data from ram while byte is non-null */
void noinline uart_puts(char *buffer);

/** output a lower bytes nibble in hex */
void noinline uart_puthexnibble(uint8_t data);

/** output a byte in hex */
void noinline uart_puthexbyte(uint8_t data);

/** output a word (16Bit) in hex */
void noinline uart_puthexword(uint16_t data);

/** output a long (32Bit) in hex */
void noinline uart_puthexlong(uint32_t data);

/** output a byte in decimal */
void noinline uart_putdecbyte(uint8_t data);

/** output eol (\r\n) */
void noinline uart_eol(void);

/* macros, for direct string output from flash */
#define uart_puts_P(s) uart_putf((void *)PSTR(s))

uint8_t uart_getline(uint8_t *line, uint8_t maxlen);

/** get one char if available in fifo **/
int	uart_getc_nowait();

/** wait until one char is available and get it **/
uint8_t uart_getc_wait();

/** get as much chars as available in fifo **/
uint8_t uart_getline_nowait(uint8_t *line, uint8_t maxlen);

#ifdef UART_USE_SUSKA
int8_t uart_send(const uint8_t *data, uint16_t size);
int8_t uart_receive(uint8_t *data, uint16_t size);
#endif	
#endif /* _UART_IRQ_H_ */
