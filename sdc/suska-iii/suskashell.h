/////////////////////////////////////
//                                 //
// Kommandozeilen-Parser           //
//                                 //
// Udo Matthe   05.01.2015         //
//                                 //
/////////////////////////////////////
#ifndef __SUSKASHELL_H__
#define __SUSKASHELL_H__

#include <avr/io.h>
#include <stdint.h>
#include "config.h"

void shell_info(void);
void shell_usb(void);
void shell_fread( uint8_t *o, uint8_t *l, uint8_t *n);
void shell_fwrite( uint8_t *o, uint8_t *l, uint8_t *n);
void shell_fdump( uint8_t *o, uint8_t *l);
void shell_ferase(void);
void shell_bftrigger(void);
void shell_sd(uint8_t *n);
void shell_ps2(void);
void shell_osd( uint8_t *o );
#ifndef SUSKA_C
void shell_ps2(void);
void shell_joy( uint8_t *level);
#endif
#endif
