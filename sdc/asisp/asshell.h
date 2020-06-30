/////////////////////////////////////
//                                 //
// Kommandozeilen-Parser           //
//                                 //
// Udo Matthe   08.12.2012         //
//                                 //
/////////////////////////////////////
#ifndef __ASSHELL_H__
#define __ASSHELL_H__

#include <avr/io.h>
#include <stdint.h>
#include "config.h"


void shell_asread(uint8_t *fname);
void shell_aswrite(uint8_t *fname);
void shell_asgetid( void);
#endif
