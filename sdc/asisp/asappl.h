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
#include "../mmc/mmc.h"
#include "../tff/ff.h"
#include "../uart-irq/uart-irq.h"


void printasstatus( void );
uint32_t getaslen( void );
void flashassave(uint8_t *fname,uint32_t fsize, uint32_t ctaddr,uint8_t endian);
void flashaswrite(uint8_t *fname, uint32_t ctaddr,uint8_t endian);

#endif
