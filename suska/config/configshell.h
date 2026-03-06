/////////////////////////////////////
//                                 //
// Config Speicher                 //
//                                 //
// Udo Matthe   05.12.2025         //
//                                 //
/////////////////////////////////////
#ifndef __CONFIGSHELL_H__
#define __CONFIGSHELL_H__

#include <avr/io.h>
#include <stdint.h>
#include "config.h"
#include <avr/eeprom.h>

extern uint8_t ee_config1 EEMEM;
extern uint8_t ee_config2 EEMEM;
void shell_cartconfig(uint8_t *para);
void shell_setconfig(uint8_t *para);
void shell_getconfig(void);
void shell_saveconfig(uint8_t *para);
void shell_bitconfig(uint8_t *bit, uint8_t *val);
void sendramconfig(void);
void initconfig(void);
#endif

