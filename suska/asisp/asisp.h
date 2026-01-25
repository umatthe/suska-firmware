/////////////////////////////////////
//                                 //
// ASISP YellowHut FPGA-Loader     //
//                                 //
//                                 //
// Udo Matthe   02.12.2012         //
//                                 //
/////////////////////////////////////

#ifndef __ASPISP__
#define __ASPISP__
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "config.h"
//#include "../timer/tick.h"
#include "../spi/spi.h"
#include "../misc/itoa.h"
#ifdef HAVE_EE_AS_ENABLE
#include <avr/eeprom.h>
#endif

#define SILICONID    0xAB
#define READ         0x03
#define WRITEENABLE  0x06
#define WRITEDISABLE 0x04
#define WRITESTATUS  0x01
#define STATUS       0x05
#define WRITE        0x02
#define ERASEBULK    0xC7
#define ERASESECTOR  0xD8
#define POWERDOWN    0xB9
#define MANID        0x9F

#define IDLETIMEOUT AS_IDLETIMEOUT

#define as_dclk_lo() (AS_PORT&=~AS_DCLK)
#define as_dclk_hi() (AS_PORT|=AS_DCLK)

#define as_cso_lo() (AS_PORT&=~AS_CSO)
#define as_cso_hi() (AS_PORT|=AS_CSO)

#define as_do_lo() (AS_PORT&=~AS_DO)
#define as_do_hi() (AS_PORT|=AS_DO)

#define as_config_lo() (AS_PORT&=~AS_CONFIG)
#define as_config_hi() (AS_PORT|=AS_CONFIG)

#define as_fpgace_lo() (AS_PORT&=~AS_FPGACE)
#define as_fpgace_hi() (AS_PORT|=AS_FPGACE)

//#ifdef AS_HAVE_LED
//#include AS_HAVE_LED
//#endif

#ifdef HAVE_EE_AS_ENABLE
extern uint8_t ee_asactive EEMEM;
#endif

uint8_t as_confdone( void );
uint8_t as_init( bool );
void as_clock( void );
void as_write( uint8_t x );
uint8_t as_read( void );
void as_writeenable( void );
void as_writedisable( void );
void as_writestatus( uint8_t x );
uint8_t as_readstatus( void );
uint8_t as_idle( void );
uint8_t as_erase( void );
void as_write256start(uint32_t addr);
void as_write256done( void );

#endif
