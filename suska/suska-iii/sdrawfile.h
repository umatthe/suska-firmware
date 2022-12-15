/////////////////////////////////////
//                                 //
// Udo Matthe   02.04.2022         //
//                                 //
/////////////////////////////////////
#ifndef __SDRAWFILE_H__
#define __SDRAWFILE_H__

#include <avr/io.h>
#include <stdint.h>
#include "config.h"
uint8_t sdraw_handle_req(void);
uint32_t sdraw_openfile(uint8_t *n, uint8_t poll);
void sdraw_closefile(void);
void sdraw_poll(void);
#endif
