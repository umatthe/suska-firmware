//
// Softuart nach:
// http://www.roboternetz.de/wissen/index.php/Software-UART_mit_avr-gcc
// 
// Benutzt: Timer1
//
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef SIGNAL
#include <avr/signal.h>
#endif // SIGNAL 

#include "softuart.h"

#ifdef _SUART_USE_FIFO_
#include "../fifo/fifo.h" 
#endif


#define nop() __asm volatile ("nop")

#ifdef SUART_TXD
    #define SUART_TXD_PORT _SUART_TX_PORT_
    #define SUART_TXD_DDR  _SUART_TX_DDR_
    #define SUART_TXD_BIT  _SUART_TX_PIN_
    static volatile uint16_t outframe;
#endif // SUART_TXD 

#ifdef SUART_RXD
    #define SUART_RXD_PORT _SUART_RX_PORT_
    #define SUART_RXD_PIN  _SUART_RX_PINREG_
    #define SUART_RXD_DDR  _SUART_RX_DDR_
    #define SUART_RXD_BIT  _SUART_RX_PIN_
    static volatile uint16_t inframe;
    static volatile uint8_t inbits, received;



    #ifdef _FIFO_H_
        #define INBUF_SIZE 4
        static uint8_t inbuf[INBUF_SIZE];
        static fifo_t infifo;
    #else // _FIFO_H_ 
        static volatile uint8_t indata;
    #endif // _FIFO_H_ 
#endif // SUART_RXD 

// Initialisierung für einen ATmega8 
// Für andere AVR-Derivate sieht dies vermutlich anders aus: 
// Registernamen ändern sich (zB TIMSK0 anstatt TIMSK, etc). 

// Uebersetzung von M8 auf M644
#ifndef TIMSK
#define TIMSK TIMSK1
#define TICIE1 ICIE1
#define TIFR TIFR1

#endif
void softuart_init(void)
{
    uint8_t tifr = 0;
    uint8_t sreg = SREG;
    cli();

#ifdef _FIFO_H_         
        fifo_init (&infifo, inbuf, INBUF_SIZE);
#endif
    // Mode #4 für Timer1 
    // und volle MCU clock 
    // IC Noise Cancel 
    // IC on Falling Edge 
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS10) | (0 << ICES1) | (1 << ICNC1);

    // OutputCompare für gewünschte Timer1 Frequenz 
    OCR1A = (uint16_t) ((uint32_t) F_CPU/_SUART_BAUDRATE_);

#ifdef SUART_RXD
    SUART_RXD_DDR  &= ~(1 << SUART_RXD_BIT);
    SUART_RXD_PORT |=  (1 << SUART_RXD_BIT);
    TIMSK |= (1 << TICIE1);
    tifr  |= (1 << ICF1) | (1 << OCF1B);
#else
    TIMSK &= ~(1 << TICIE1);
#endif // SUART_RXD 

#ifdef SUART_TXD
    tifr |= (1 << OCF1A);
    SUART_TXD_PORT |= (1 << SUART_TXD_BIT);
    SUART_TXD_DDR  |= (1 << SUART_TXD_BIT);
    outframe = 0;
#endif // SUART_TXD 

    TIFR = tifr;

    SREG = sreg;
}

#ifdef  SUART_TXD
void softuart_putc (const uint8_t c)
{
    do
    {
        sei(); nop(); cli(); // yield(); 
    } while (outframe);

    // frame = *.P.7.6.5.4.3.2.1.0.S   S=Start(0), P=Stop(1), *=Endemarke(1) 
    outframe = (3 << 9) | (((uint8_t) c) << 1);

    TIMSK |= (1 << OCIE1A);
    TIFR   = (1 << OCF1A);

    sei();
}

SIGNAL (TIMER1_COMPA_vect)
{
    uint16_t data = outframe;
   
    if (data & 1)      SUART_TXD_PORT |=  (1 << SUART_TXD_BIT);
    else               SUART_TXD_PORT &= ~(1 << SUART_TXD_BIT);
   
    if (1 == data)
    {
        TIMSK &= ~(1 << OCIE1A);
    }   
   
    outframe = data >> 1;
}
#endif // SUART_TXD

#ifdef SUART_RXD
SIGNAL (TIMER1_CAPT_vect)
{
    uint16_t icr1  = ICR1;
    uint16_t ocr1a = OCR1A;
   
    // Eine halbe Bitzeit zu ICR1 addieren (modulo OCR1A) und nach OCR1B
    uint16_t ocr1b = icr1 + ocr1a/2;
    if (ocr1b >= ocr1a)
        ocr1b -= ocr1a;
    OCR1B = ocr1b;
   
    TIFR = (1 << OCF1B);
    TIMSK = (TIMSK & ~(1 << TICIE1)) | (1 << OCIE1B);
    inframe = 0;
    inbits = 0;
}

SIGNAL (TIMER1_COMPB_vect)
{
    uint16_t data = inframe >> 1;
   
    if (SUART_RXD_PIN & (1 << SUART_RXD_BIT))
        data |= (1 << 9);
      
    uint8_t bits = inbits+1;
   
    if (10 == bits)
    {
        if ((data & 1) == 0)
            if (data >= (1 << 9))
            {
#ifdef _FIFO_H_         
                _inline_fifo_put (&infifo, data >> 1);
#else            
                indata = data >> 1;
#endif // _FIFO_H_            
                received = 1;
            }
      
        TIMSK = (TIMSK & ~(1 << OCIE1B)) | (1 << TICIE1);
        TIFR = (1 << ICF1);
    }
    else
    {
        inbits = bits;
        inframe = data;
    }
}

#ifdef _FIFO_H_

int8_t softuart_getc_wait(void)
{
    return (int8_t) fifo_get_wait (&infifo);
}

int8_t softuart_getc_nowait(void)
{
    return (int8_t) fifo_get_nowait (&infifo);
}

#else // _FIFO_H_

int8_t softuart_getc_wait(void)
{
    while (!received)   {}
    received = 0;
   
    return (int8_t) indata;
}

int8_t softuart_getc_nowait(void)
{
    if (received)
    {
        received = 0;
        return (int8_t) indata;
    }
   
    return -1;
}

#endif // _FIFO_H_
#endif // SUART_RXD

