#ifndef _MMCHW_H_
#define _MMCWH_H_

void mmc_hwinit(void);

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

#define MMC_Disable() _MMC_Write_|=  _BV(_MMC_Chip_Select_)
#define MMC_Enable()  _MMC_Write_&= ~_BV(_MMC_Chip_Select_)

#ifdef SUSKA_BF 
#define MMC_Off()   _MMC_CTRL_PORT_|=  _BV(_MMC_ACTIVATE_); // uart_puts_P("SD-OFF\n\r");
#define MMC_On()  _MMC_CTRL_PORT_&= ~_BV(_MMC_ACTIVATE_);  //uart_puts_P("SD-ON\n\r");
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

#endif //_MMC_H_
