#include <avr/io.h>
#include "config.h"
#include "spi.h"

uint8_t slowspi=0; //Dummy not used for HW-SPI

void spi_enable_slow(void)
{
    /* enable spi, set master and clock modes (f/8) */
    _SPCR0 = _BV(_SPE0) | _BV(_MSTR0) | _BV(SPR1);
    _SPSR0 = _BV(_SPI2X0);
}

void spi_enable(void)
{
#if (F_CPU==48000000UL) | (F_CPU==56000000UL) | (F_CPU==60000000UL) | (F_CPU==64000000UL)
    /* enable spi, set master and clock modes (f/8) */
//    _SPCR0 = _BV(_SPE0) | _BV(_MSTR0) | _BV(SPR1);
//    _SPSR0 = _BV(_SPI2X0);

    /* enable spi, set master and clock modes (f/4) */
    _SPCR0 = _BV(_SPE0) | _BV(_MSTR0);
    _SPSR0 = 0;

    /* enable spi, set master and clock modes (f/2) */
//    _SPCR0 = _BV(_SPE0) | _BV(_MSTR0);
//    _SPSR0 = _BV(_SPI2X0);
#else
    /* enable spi, set master and clock modes (f/2) */
    _SPCR0 = _BV(_SPE0) | _BV(_MSTR0);
    _SPSR0 = _BV(_SPI2X0);
#endif
}

void spi_init(void)
{

    /* configure MOSI, SCK lines as outputs */
    //
    // SPI_SS will be set to output 
    // It can be used for User-I/O (as output)
    // Using it as Input would disturb SPI-Master-Mode
    //
#ifndef AVRCORE
    SPI_DDR |= _BV(SPI_MOSI) | _BV(SPI_SCK) | _BV(SPI_SS);
#endif
    spi_enable();
}


void noinline spi_wait_busy(void)
/* {{{ */ {

#   ifdef SPI_TIMEOUT
    uint8_t timeout = 200;

    while (!(_SPSR0 & _BV(_SPIF0)) && timeout > 0)
        timeout--;

    if (timeout == 0)
        uart_puts_P("ERROR: spi timeout reached!\r\n");
#   else
    while (!(_SPSR0 & _BV(_SPIF0)));
#   endif

} /* }}} */

uint8_t noinline spi_send(uint8_t data)
/* {{{ */ {

    _SPDR0 = data;
    spi_wait_busy();

    return _SPDR0;

} /* }}} */


#ifdef SPI_USE_CHUNK
void SpiWriteChunk(     const uint8_t* pChunk,
                                        uint16_t nBytes)
{
        while (nBytes--)
        {
                SPDR = *pChunk++;
                while (!(SPSR & (1<<SPIF)))
                {
                        asm volatile ("nop");
                }
        }
}

void SpiReadChunk(      uint8_t* pChunk,
                                        uint16_t nBytes)
{
        while (nBytes--)
        {
                SPDR = 0;
                while (!(SPSR & (1<<SPIF)))
                {
                        asm volatile ("nop");
                }

                *pChunk++ = SPDR;
        }

}
#endif
