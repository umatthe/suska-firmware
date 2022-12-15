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

#include "mmc.h"
#include "softspi.h"



////////////////////////////
// Initialize mmc/sd card //
////////////////////////////
void mmc_hwinit( void )
{
	MMC_Disable();
	MMC_Off();
	MMC_INH_OFF();
        Softspi_cs( 0 );
}
