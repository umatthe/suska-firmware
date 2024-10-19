////////////////////////////////////////////////////////////
//
// mmc/sd library for Atmega 
//
// Based on mmc.c/h from Ulrich Radig (www.ulichradig.de)
// and   on mmc.c/h from ChaN (elm-chan.org)      
//
// Adopted/stripped/enhanced by Udo Matthe
//
// Depends on:
// - Etherrape spi library
// - Etherrape uart library for debuging
// - tick.c/h (delayms)
//
////////////////////////////////////////////////////////////

#include "config.h"
#include "mmc.h"



////////////////////////////
// Initialize mmc/sd card //
////////////////////////////
void mmc_hwinit( void )
{
	MMC_Disable();
	MMC_Off();
	MMC_INH_OFF();

#ifdef _MMC_CTRL_DDR_
	 _MMC_CTRL_DDR_  |= _BV(_MMC_ACTIVATE_);
//	 _MMC_CTRL_PORT_ &= ~_BV(_MMC_ACTIVATE_); // Suska-BF Low = PowerOn (will be switched on in mmc_init) 
#endif
        _MMC_DDR_ |= _BV(_MMC_Chip_Select_);
}
