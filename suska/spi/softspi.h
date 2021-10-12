#ifndef _SOFTSPI_H
#define _SOFTSPI_H

#include <stdint.h>

#define noinline __attribute__((noinline))


#ifdef SPI_USE_CHUNK
void SoftSpiWriteChunk(const uint8_t* pChunk, uint16_t nBytes);
void SoftSpiReadChunk(uint8_t* pChunk, uint16_t nBytes);
#endif
// Dummys only in HW-SPI used
#define Softspi_enable() 
#define Softspi_enable_slow() 
#define Softspi_disable() 
#define Softspi_wait_busy() 

void Softspi_init(void);
uint8_t noinline Softspi_send(uint8_t data);
// Aliases 
#define SoftSpiWriteByte(x) spi_send(x)
#define SoftSpiReadByte() spi_send(0)
#endif
