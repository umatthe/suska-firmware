#ifndef _MMCHW_H_
#define _MMCWH_H_
#include "softspi.h"

#define USE_SoftSPI

void mmc_hwinit( void );

#define MMC_Disable() Softspi_cs(0)
#define MMC_Enable() Softspi_cs(1)

#define MMC_On() 
#define MMC_Off()

#define MMC_INH_ON()
#define MMC_INH_OFF()

#endif //_MMC_H_
