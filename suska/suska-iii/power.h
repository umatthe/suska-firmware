#ifndef __POWER_H__
#define __POWER_H__
#include <stdint.h>

#include "config.h"


void power_resetpin_init(void);
void power_restart(void);
void power_fboot( void );
void set_reset_pin(uint8_t on);
void set_reset_core_pin(uint8_t on);
void shell_res( uint8_t *level);
void shell_cres( uint8_t *level);
#ifndef SUSKA_C
void power_init(void);
uint8_t switch_power(uint8_t on);
#endif
#endif
