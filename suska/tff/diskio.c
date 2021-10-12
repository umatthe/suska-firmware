#include "config.h"
#include "diskio.h"
#include "../spi/spi.h"
#include "../mmc/mmc.h"

#ifdef _PFF_
/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
	DSTATUS stat;

	if(!mmc_init()) 
		stat = STA_NODISK|STA_NOINIT;
	else
		stat = 0;

	return stat;
}
/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	BYTE* dest,			/* Pointer to the destination object */
	DWORD sector,		/* Sector number (LBA) */
	WORD sofs,			/* Offset in the sector */
	WORD count			/* Byte count (bit15:destination) */
)
{

	return mmc_read_part(sector,dest,sofs,count);
}


#ifdef _PFF_WRITE_FUNC_
/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_writep (
	const BYTE* buff,		/* Pointer to the data to be written, NULL:Initiate/Finalize write operation */
	DWORD sc		/* Sector number (LBA) or Number of bytes to send */
)
{
	return mmc_write_part(buff,sc);
}
#endif

#else  // Full FF
static DSTATUS stat;

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber */
)
{
                if(!mmc_init()) 
			stat = STA_NODISK|STA_NOINIT;
		else
		   	stat = 0;

		return stat;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber */
)
{
		return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector number (LBA) */
	BYTE count		/* Sector count (1..255) */
)
{
		return mmc_read_sector(sector, buff);   // Note: count must not exeed 1 !!!

}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
		return mmc_write_sector(sector, buff); // Note: count must not exeed 1 !!!

}
#endif /* _READONLY */
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;
        uint8_t *ptr = buff;



		MMC_Enable();		/* CS = L */

		switch (ctrl) {

		case CTRL_SYNC :	/* Make sure that data has been written */
			if (mmc_wait_ready() == 0xFF)
				res = RES_OK;
			break;

		case MMC_GET_TYPE :             /* Get card type flags (1 byte) */
			*ptr = mmc_get_CardType();
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
		}

		MMC_Disable();			/* CS = H */
		spi_send(0xff);			/* Idle (Release DO) */




		return res;

}
#endif
