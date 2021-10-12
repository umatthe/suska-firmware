#ifndef _SPIUART_H
#define _SPIUART_

#include <stdint.h>

#define noinline __attribute__((noinline))

void spiuart_init(uint16_t baud);
uint8_t noinline spiuart_send(uint8_t data);

// Aliases 
#define SpiuartWriteByte(x) spiuart_send(x)
#define SpiuartReadByte() spiuart_send(0)

#ifdef SPI_USE_CHUNK
void SpiuartWriteChunk(const uint8_t* pChunk, uint16_t nBytes);
void SpiuartReadChunk(uint8_t* pChunk, uint16_t nBytes);
#endif

#endif // _SPIUART_H
