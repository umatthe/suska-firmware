/////////////////////////////////////
//                                 //
// Main File YellowHut FPGA-Loader //
//                                 //
// Für Suska-FPGA Board o.ae.      //
//                                 //
// Udo Matthe   02.12.2012         //
//                                 //
/////////////////////////////////////

#ifndef __COMMANDS__
#define __COMMANDS__
#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "config.h"
#ifdef __HAVE_FILESYSTEM__
#include "../tff/ff.h"
#include "../mmc/mmc.h"
#endif
#include "../uart-irq/uart-irq.h"
#include "../misc/itoa.h"

#define LINELEN 80
extern uint8_t fsline[];
extern uint32_t tracelevel;
#ifdef __HAVE_FILESYSTEM__
FRESULT show_file(char *filename,uint8_t channel);
FRESULT hexdump_file(char *filename,uint8_t channel);
FRESULT show_dir(int8_t channel);
void fserr_out(FRESULT res);
uint8_t* readline ( uint8_t* buff, uint8_t len, FIL* fil );
#endif
char *strtok(char *s, const char *delim);
#endif
