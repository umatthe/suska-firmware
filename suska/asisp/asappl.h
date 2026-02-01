/////////////////////////////////////
//                                 //
// ASISP YellowHut FPGA-Loader     //
//                                 //
//                                 //
// Udo Matthe   02.12.2012         //
//                                 //
/////////////////////////////////////

#ifndef __ASAPPL__
#define __ASAPPL__
#include "asisp.h"
#ifdef __HAVE_FILESYSTEM__
#include "../mmc/mmc.h"
#include "../tff/ff.h"
#endif
#include "../uart-irq/uart-irq.h"


void printasstatus( void );
uint8_t getasid( void );
uint32_t getaslen( void );
uint32_t checkaslen( void );
void powerdownas( uint8_t en );
#ifdef __HAVE_FILESYSTEM__
void flashassave(uint8_t *fname,uint32_t fsize, uint32_t ctaddr,uint8_t endian);
void flashaswrite(uint8_t *fname, uint32_t ctaddr,uint8_t endian);
#endif
#endif
