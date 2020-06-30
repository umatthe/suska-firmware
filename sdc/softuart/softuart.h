#ifndef _SOFTUART_H_
#define _SOFTUART_H_
#include "config.h"

#if _SUART_TXD_ == 1
#define SUART_TXD
#elif _SUART_TXD_ != 0
#warning "_SUART_TXD != 1|0 using default _SUART_TXD_=1"
#define SUART_TXD
#endif

#if _SUART_RXD_ == 1
#define SUART_RXD
#elif _SUART_RXD_ != 0
#warning "_SUART_RXD != 1|0 using default _SUART_RXD_=1"
#define SUART_RXD
#endif

#ifndef _SUART_USE_FIFO_
#ifndef _SUART_NO_FIFO_
#warning "_SUART_USE_FIFO_ and _SUART_NO_FIFO_ undefined using default NO_FIFO"
#endif
#endif

#ifndef _SUART_TX_PORT_
#define _SUART_TX_PORT_ PORTD
#warning "_SUART_TX_PORT_ not defined (using PORTD)"
#endif

#ifndef _SUART_TX_DDR_
#define _SUART_TX_DDR_ DDRD
#warning "_SUART_TX_DDR_ not defined (using DDRD)"
#endif

#ifndef _SUART_TX_PIN_
#define _SUART_TX_PIN_ PD7
#warning "_SUART_TX_PIN_ not defined (using PD7)"
#endif

#ifndef _SUART_RX_PORT_
#define _SUART_RX_PORT_ PORTD
#warning "_SUART_RX_PORT_ not defined (using PORTD)"
#endif

#ifndef _SUART_RX_DDR_
#define _SUART_RX_DDR_ DDRD
#warning "_SUART_RX_DDR_ not defined (using DDRD)"
#endif

#ifndef _SUART_RX_PIN_
#define _SUART_RX_PIN_ PD6
#warning "_SUART_RX_PIN_ not defined (using PD6)"
#endif

#ifndef _SUART_RX_PINREG_
#define _SUART_RX_PINREG_ PIND
#warning "_SUART_RX_PINREG_ not defined (using PIND)"
#endif

#if defined( __AVR_ATmega8__) && (_SUART_RX_PIN_!=PB0)
#warning "ICP1 (RX_PIN) of Atmega8 should be PB0  softuart RX may not work"
#elif defined( __AVR_ATmega644__) && (_SUART_RX_PIN_!=PD6)
#warning "ICP1 (RX_PIN) of Atmega644 should be PD6  softuart RX may not work"
#elif (defined(__AVR_ATmega649__) || (__AVR_ATmega649A__)) && (_SUART_RX_PIN_!=PD0)
#warning "ICP1 (RX_PIN) of Atmega649(A) should be PD0  softuart RX may not work"
#endif

#ifndef _SUART_BAUDRATE_
#define _SUART_BAUDRATE_ 19200
#warning "_SUART_BAUDRATE_ not defined (using 19200)"
#endif

void softuart_init(void);

#ifdef SUART_TXD
    void softuart_putc (const uint8_t);
#endif // SUART_RXD

#ifdef SUART_RXD
    int8_t softuart_getc_wait(void);
    int8_t softuart_getc_nowait(void);
#endif // SUART_RXD

#endif /* _SOFTUART_H_ */

