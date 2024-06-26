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
#include "../spi/spi.h"
//#ifdef DEBUGMMC
#include "../uart-irq/uart-irq.h"
#include "../misc/itoa.h"
//#endif

#include "mmc.h"
//#include "../timer/tick.h"
#include <util/delay.h>


#include <util/crc16.h>

/* Definitions for MMC/SDC command */
#define CMD0    (0x40+0)        /* GO_IDLE_STATE */
#define CMD1    (0x40+1)        /* SEND_OP_COND (MMC) */
#define ACMD41  (0xC0+41)       /* SEND_OP_COND (SDC) */
#define CMD8    (0x40+8)        /* SEND_IF_COND */
#define CMD9    (0x40+9)        /* SEND_CSD */
#define CMD10   (0x40+10)       /* SEND_CID */
#define CMD12   (0x40+12)       /* STOP_TRANSMISSION */
#define ACMD13  (0xC0+13)       /* SD_STATUS (SDC) */
#define CMD16   (0x40+16)       /* SET_BLOCKLEN */
#define CMD17   (0x40+17)       /* READ_SINGLE_BLOCK */
#define CMD18   (0x40+18)       /* READ_MULTIPLE_BLOCK */
#define CMD23   (0x40+23)       /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0xC0+23)       /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (0x40+24)       /* WRITE_BLOCK */
#define CMD25   (0x40+25)       /* WRITE_MULTIPLE_BLOCK */
#define CMD55   (0x40+55)       /* APP_CMD */
#define CMD58   (0x40+58)       /* READ_OCR */

static uint8_t CardType;                  /* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */

uint8_t mmc_get_CardType( void )
{
	return CardType;
}

//////////////
// Send Cmd //
//////////////
uint8_t mmc_write_command ( uint8_t cmd,  uint32_t arg )
{
	uint8_t n, res;
	uint16_t retry;

#ifdef DEBUGMMC
	uart_puts_P("UMA: mmc cmd: ");
	uart_puthexbyte((cmd));
	uart_puts_P(" : ");
	for(uint8_t i=0;i<4;i++)
	{
		uart_puthexbyte((arg>>(i*8))&0xff);
	}
	uart_eol();
#endif

	if (cmd & 0x80) 
	{       /* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = mmc_write_command(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	MMC_Disable();
	MMC_Enable();
	if (mmc_wait_ready() != 0xFF) return 0xFF;

#ifdef DEBUGMMC
	uart_puts_P("UMA: send cmd\r\n");
#endif
	/* Send command packet */
	spi_send(cmd);                             /* Start + Command index */
	spi_send((uint8_t)(arg >> 24));            /* Argument[31..24] */
	spi_send((uint8_t)(arg >> 16));            /* Argument[23..16] */
	spi_send((uint8_t)(arg >> 8));             /* Argument[15..8] */
	spi_send((uint8_t)arg);                    /* Argument[7..0] */
	n = 0x01;                                  /* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;                 /* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;                 /* Valid CRC for CMD8(0x1AA) */
	spi_send(n);

	/* Receive command response */
	if (cmd == CMD12) spi_send(0xff);           /* Skip a stuff byte when stop reading */
	retry = 500;                                /* Wait for a valid response in timeout of 500 attempts */
	do
		res = spi_send(0xff);
	while ((res & 0x80) && --retry);

#ifdef DEBUGMMC
	uart_puts_P("UMA: return: ");
	uart_puts(itoa(res));
	uart_puts_P(" after retries: ");
	uart_puts(itoa(retry));
	uart_eol();
#endif
	return res;                     /* Return with the response value */
}

////////////////////////////
// Initialize mmc/sd card //
////////////////////////////
void mmc_hwinit( void )
{
	MMC_Disable();
	MMC_Off();
	MMC_INH_OFF();
#ifdef _MMC_CTRL_
         _MMC_CTRL_DDR_ |= _BV(_MMC_ACTIVATE_);
#else
#ifdef _MMC_CTRL_DDR_
	 _MMC_CTRL_DDR_  |= _BV(_MMC_ACTIVATE_);
//	 _MMC_CTRL_PORT_ &= ~_BV(_MMC_ACTIVATE_); // Suska-BF Low = PowerOn
#endif
#endif
        _MMC_DDR_ |= _BV(_MMC_Chip_Select_);
}

uint8_t mmc_init( void )
{
	uint16_t retry;
	uint8_t n, cmd, ty, ocr[4];
#ifndef EN_PS
        mmc_hwinit();	
#else
        MMC_On();
	MMC_INH_ON();
#endif
        _delay_ms(100);

	// Send 128 clk
	for (uint8_t b = 0;b<0x0f;b++)
	{
		spi_send(0xff);
	}
	
        _delay_ms(100);

        retry=0;
	ty = 0;

	uint8_t cmd0=1;
        while(mmc_write_command (CMD0,0) !=1)
        {
                if (retry++ > 10)
                {
			cmd0=0;
			break;
                }
                _delay_ms(50);
        }

	if (cmd0 == 1) 
	{    /* Enter Idle state */
		if (mmc_write_command(CMD8, 0x1AA) == 1) 
		{       /* SDHC */
			for (n = 0; n < 4; n++) ocr[n] = spi_send(0xff);            /* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) 
			{   /* The card can work at vdd range of 2.7-3.6V */
				while ((retry++ < 500) && mmc_write_command(ACMD41, 1UL << 30))
				{
				  _delay_ms(50);  /* Wait for leaving idle state (ACMD41 with HCS bit) */
  				}
				if ((retry < 500) && mmc_write_command(CMD58, 0) == 0) 
				{ /* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = spi_send(0xff);
					uart_puts_P("OCR :");
					uart_puthexbyte(ocr[0]); uart_puts_P(" ");
					uart_puthexbyte(ocr[1]); uart_puts_P(" ");
					uart_puthexbyte(ocr[2]); uart_puts_P(" ");
					uart_puthexbyte(ocr[3]); uart_puts_P(" ");
					uart_eol();
					ty = (ocr[0] & 0x40) ? 12 : 4;
				}
			}
		} 
		else 
		{     /* SDSC or MMC */
			if (mmc_write_command(ACMD41, 0) <= 1)   
			{
				ty = 2; cmd = ACMD41;   /* SDSC */
			} 
			else 
			{
				ty = 1; cmd = CMD1;     /* MMC */
			}
			while ((retry++ < 1000) && mmc_write_command(cmd, 0));           /* Wait for leaving idle state */
			if ((retry == 1000) || mmc_write_command(CMD16, 512) != 0)       /* Set R/W block length to 512 */
				ty = 0;
		}
	}
	CardType = ty;
	
	// set MMC_Chip_Select to high
	MMC_Disable();

	if (ty) 
	{  
		// Init succeeded:	
//#ifdef DEBUGMMC
		uart_puts_P("CardType :");
		uart_puthexbyte(ty);
		uart_eol();
//#endif
		return(1);
	}
	else
	{
//#ifdef DEBUGMMC
		uart_puts_P("mmc_init Error");
		uart_eol();
//#endif
		MMC_Off();
		return(0);
	}
}

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

uint8_t mmc_wait_ready (void)
{
	uint8_t res;
        uint16_t retry=500;

	spi_send(0xff);
	do
		res = spi_send(0xff);
	while ((res != 0xFF) && retry--);

#ifdef DEBUGMMC
	uart_puts_P("UMA: ready after retries: ");
	uart_puts(itoa(retry));
	uart_eol();
#endif
	return res;
}


#ifndef _PFF_
//////////////////////////
// Write 512 Byte block //
//////////////////////////
uint8_t mmc_write_sector (uint32_t blocknr,const uint8_t *Buffer)
{
	uint16_t crc16calc;
	uint8_t tmp;

	// Calculate Byte Position from blocknr 
	if (!(CardType & 8)) blocknr = blocknr << 9;    /* Convert to byte address if needed */

	if((tmp = mmc_write_command (CMD24,blocknr)) != 0)
	{
		return tmp;
	}

        if (mmc_wait_ready() != 0xFF) return 1;  // RW-Error

	//Send start transfer 
	spi_send(0xfe);	

	// Send Data	
	for (uint16_t a=0;a<512;a++)
	{
		spi_send(Buffer[a]);
		crc16calc=_crc_xmodem_update(crc16calc,Buffer[a]);
	}

	// send CRC word
	spi_send(crc16calc>>8); 
	spi_send(crc16calc&0xff); 

	// Check Response
        tmp = spi_send(0xff); 
	if ((tmp & 0x1F) != 0x05)   return 1;  // RW-Error

	// Wait to complete transaction
	while (spi_send(0xff) != 0xff);

	//set MMC_Chip_Select to high 
	MMC_Disable();

	return 0;
}

/////////////////////////
// Read 512 Byte Block //
/////////////////////////
uint8_t mmc_read_sector (uint32_t blocknr, uint8_t *Buffer)
{	
	uint8_t tmp;
	uint16_t crc;
        uint16_t crc16calc=0;
	uint8_t cmd[] = {0x51,0x00,0x00,0x00,0x00,0xFF};     // Read cmd


	// Calculate Byte Position from blocknr 
	if (!(CardType & 8)) blocknr = blocknr << 9;    /* Convert to byte address if needed */

	if((tmp = mmc_write_command (CMD17,blocknr)) != 0)
	{
//#ifdef DEBUGMMC
	        uart_puts_P(" fail write cmd\r\n");
//#endif
		return tmp;
	}

        // Wait for start transfer	
	while (spi_send(0xff) != 0xfe);

	// Read data
	for (uint16_t a=0;a<512;a++)
	{
		Buffer[a] = spi_send(0xff);
		crc16calc=_crc_xmodem_update(crc16calc,Buffer[a]);
	}
	// read CRC word
	crc=spi_send(0xff)<<8;
	crc|=spi_send(0xff);

#ifdef DEBUGMMC
	uart_puts_P("UMA: mmc crc: ");
	uart_puthexbyte(crc>>8);
	uart_puthexbyte(crc&0xff);
	uart_puts_P(" calc crc: ");
	uart_puthexbyte(crc16calc>>8);
	uart_puthexbyte(crc16calc&0xff);
#endif
	//set MMC_Chip_Select to high
	MMC_Disable();
        if(crc == crc16calc)
	{
#ifdef DEBUGMMC
	        uart_puts_P(" ok\r\n");
#endif
		return 0;
	}
	else
	{
#ifdef DEBUGMMC
	        uart_puts_P(" failed\r\n");
#endif
		return 1;
	}
}


void mmc_dump_buffer(uint8_t *buffer,uint16_t len)
{
#ifdef DEBUGMMC
	uint8_t asci[16];
	uint8_t index=0;

	for (uint16_t i = 0; i < len; i++)
	{
		uart_putc(' ');
		uart_puthexbyte(buffer[i]);
		if(buffer[i]>=32 && buffer[i]<127)
		{
			asci[index]=buffer[i];
		}
		else
		{
			asci[index]='.';
		}
		index++;
		if(!((i+1)%16))
		{
			for(uint8_t x=0;x<index;x++) uart_putc(asci[x]);
			index=0;
			uart_eol();
		}
	}
	uart_eol();
#endif
}
#else // _PFF_
/////////////////////////////
// Read partial Byte Block //
/////////////////////////////
uint8_t mmc_read_part (uint32_t blocknr, uint8_t *Buffer,uint16_t ofs, uint16_t cnt)
{	
	uint8_t tmp;
	uint8_t cmd[] = {0x51,0x00,0x00,0x00,0x00,0xFF};     // Read cmd
	uint16_t bc;


	// Calculate Byte Position from blocknr 
	if (!(CardType & 8)) blocknr = blocknr << 9;    /* Convert to byte address if needed */

	if((tmp = mmc_write_command (CMD17,blocknr)) != 0)
	{
		return 1;
	}

        // Wait for start transfer	
	while (spi_send(0xff) != 0xfe);

	// Read data
	bc = 514 - ofs - cnt;
	/* Skip leading bytes */
        if(ofs)
	{
		do spi_send(0xff); while(--ofs);
	}
	/* Receive a part of the sector */
	if(Buffer)
	{
		do *Buffer++ = spi_send(0xff); while(--cnt);
	}
	/* Skip trailing bytes and CRC */
	do spi_send(0xff); while(--bc);
	
	//set MMC_Chip_Select to high
	MMC_Disable();
	return 0;
}

#ifdef _PFF_WRITE_FUNC_
/*-----------------------------------------------------------------------*/
/* Write partial sector                                                  */
/*-----------------------------------------------------------------------*/

uint8_t mmc_write_part (
	const uint8_t *buff,	/* Pointer to the bytes to be written (NULL:Initiate/Finalize sector write) */
	uint32_t blocknr	/* Number of bytes to send, Sector number (LBA) or zero */
)
{
	uint8_t res;
	uint16_t bc;
	static uint16_t wc;


	res = 1; // Error

	if (buff) 
	{		/* Send data bytes */
		bc = (uint16_t)blocknr;
		while (bc && wc) 
		{		/* Send data bytes to the card */
			spi_send(*buff++);
			wc--; bc--;
		}
		res = 0;
	} 
	else 
	{
		if (blocknr) 
		{	/* Initiate sector write process */
			// Calculate Byte Position from blocknr 
			if (!(CardType & 8)) blocknr = blocknr << 9;    /* Convert to byte address if needed */

			if((res = mmc_write_command (CMD24,blocknr)) != 0)
			{
				return res;
			}

			if (mmc_wait_ready() != 0xFF) return 1;  // RW-Error

			//Send start transfer 
			spi_send(0xfe);	
			wc = 512;							/* Set byte counter */
			res = 0;
		} 
		else 
		{	/* Finalize sector write process */
			bc = wc + 2;
			while (bc--) spi_send(0);	/* Fill left bytes and CRC with zeros */

			// Check Response
			res = spi_send(0xff); 
			if ((res & 0x1F) != 0x05)   return 1;  // RW-Error

			// Wait to complete transaction
			while (spi_send(0xff) != 0xff);

			//set MMC_Chip_Select to high 
			MMC_Disable();
			res=0;
		}
	}

	return res;
}
#endif
#endif
