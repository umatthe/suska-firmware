#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>

#define noinline __attribute__((noinline))

#ifndef SPCR0
// New avrlib (1.4.6 ++)
#define SPCR0  SPCR
#define SPIE0  SPIE 
#define SPE0   SPE
#define DORD0  DORD
#define MSTR0  MSTR
#define CPOL0  CPOL
#define CPHA0  CPHA
#define SPR01  SPR1
#define SPR00  SPR0

#define SPSR0  SPSR
#define SPIF0  SPIF
#define WCOL0  WCOL
#define SPI2X0 SPI2X

#ifndef SPDR0
#define SPDR0  SPDR 
#endif
#endif


/* prototypes */


#ifndef USE_SoftSPI

#ifdef SPI_USE_CHUNK
void SpiWriteChunk(const uint8_t* pChunk, uint16_t nBytes);
void SpiReadChunk(uint8_t* pChunk, uint16_t nBytes);
#endif
void spi_enable(void);
void spi_enable_slow(void);
#define spi_disable() _SPCR0 = 0
void noinline spi_wait_busy(void);
void spi_init(void);
uint8_t noinline spi_send(uint8_t data);
// Aliases 
#define SpiWriteByte(x) spi_send(x)
#define SpiReadByte() spi_send(0)

#else
#include "softspi.h"

#define spi_enable() Softspi_enable() 
#define spi_enable_slow() Softspi_enable_slow() 
#define spi_disable() Softspi_disable() 
#define spi_wait_busy() Softspi_wait_busy() 

#define spi_init() Softspi_init()
#define spi_send(x) Softspi_send(x)

#define SpiWriteByte(x) Softspi_send(x)
#define SpiReadByte() Softspi_send(0)
#ifdef SPI_USE_CHUNK
#define SpiWriteChunk(x,y) SoftSpiWriteChunk(x,y)
#define SpiReadChunk(x,y) SoftSpiReadChunk(x,y)
#endif
#endif // USE_SoftSPI
#endif
