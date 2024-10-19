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

#ifdef __HAVE_FILESYSTEM__
void shell_asread(uint8_t *fname);
void shell_aswrite(uint8_t *fname);
#endif
void shell_asgetid(void);
void shell_aspowerdown(void);
#ifdef HAVE_EE_AS_ENABLE
void shell_asenable(uint8_t *val);
#endif
#endif
