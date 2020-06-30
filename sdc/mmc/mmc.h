#ifndef _MMC_H_
#define _MMC_H_

#include <avr/io.h>
#include <stdint.h>

#include "config.h"

#ifndef _MMC_Write_
#define _MMC_Write_        PORTB	// CS Port 
#warning "_MMC_Write_ not defined using Default PORTB"
#endif
#ifndef _MMC_DDR_
#define _MMC_DDR_  DDRB	
#warning "_MMC_DDR_ not defined using Default DDR"
#endif
#ifndef _MMC_Chip_Select_
#define _MMC_Chip_Select_  PB0	// CS Pin
#warning "_MMC_Chip_Select_ not defined using Default PB0"
#endif

#ifdef _MMC_CTRL_
#ifndef _MMC_CTRL_DDR_ 
#error "_MMC_CTRL_ defined _MMC_CTRL_DDR_ needs to be defined too" 
#endif
#ifndef _MMC_ACTIVATE_ 
#error "_MMC_CTRL_ defined _MMC_ACTIVATE_ needs to be defined too" 
#endif
#ifndef _MMC_CDI_ 
#error "_MMC_CTRL_ defined _MMC_CDI_ needs to be defined too" 
#endif
#ifndef _MMC_WP_ 
#error "_MMC_CTRL_ defined _MMC_WP_ needs to be defined too" 
#endif
#endif

void mmc_hwinit(void);

uint8_t mmc_init(void);

uint8_t mmc_read_sector (uint32_t ,uint8_t *);

uint8_t mmc_write_sector (uint32_t ,const uint8_t *);

uint8_t mmc_write_command (uint8_t cmd, uint32_t arg);

uint8_t mmc_wait_ready(void);

uint8_t mmc_get_CardType(void);

void mmc_dump_buffer(uint8_t * ,uint16_t);

#define MMC_Disable() _MMC_Write_|=  _BV(_MMC_Chip_Select_)
#define MMC_Enable()  _MMC_Write_&= ~_BV(_MMC_Chip_Select_)

#ifdef _MMC_CTRL_
#define MMC_On()   _MMC_CTRL_|=  _BV(_MMC_ACTIVATE_)
#define MMC_Off()  _MMC_CTRL_&= ~_BV(_MMC_ACTIVATE_)
#elif defined SUSKA_BF // _MMC_DDR_
#define MMC_Off()   _MMC_CTRL_PORT_|=  _BV(_MMC_ACTIVATE_);  uart_puts_P("SD-OFF\n\r");
#define MMC_On()  _MMC_CTRL_PORT_&= ~_BV(_MMC_ACTIVATE_);  uart_puts_P("SD-ON\n\r");
#else
#define MMC_On() 
#define MMC_Off()
#endif

#ifdef _MMC_INH_
#define MMC_INH_ON()  _MMC_INH_PORT_|= _BV(_MMC_INH_); _MMC_INH_DDR_|= _BV(_MMC_INH_)
#define MMC_INH_OFF() _MMC_INH_PORT_&= ~_BV(_MMC_INH_); _MMC_INH_DDR_|= _BV(_MMC_INH_)
#else
#define MMC_INH_ON()
#define MMC_INH_OFF()
#endif
// PFF Interfacces
uint8_t mmc_read_part (uint32_t blocknr, uint8_t *Buffer,uint16_t ofs, uint16_t cnt);
uint8_t mmc_write_part ( const uint8_t *buff, uint32_t blocknr);

#endif //_MMC_H_
