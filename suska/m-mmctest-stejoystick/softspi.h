#ifndef _SOFTSPI_H
#define _SOFTSPI_H

#include <stdint.h>
#define _BV(bit) (1 << (bit))

#define noinline __attribute__((noinline))


void Softspi_init(void);
uint8_t noinline Softspi_send(uint8_t data);
void Softspi_cs(uint8_t active);
#endif
