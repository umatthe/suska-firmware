#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "suskaspi.h"
#include "buttons.h"

//#define SUSKASPI_SLOW

#ifdef SHELL_POLL
#include "../shell/shell.h"
#endif


void Suskaspi_init(void)
{
#ifdef BOOT_SD_AVR_EN
            /* configure and initialize AVR_SD_ENn */
            /* Currently only Suska-C to select FBOOT from SDC instead of Sysboot-AVR */
            BOOTAVR_DISABLE;
            BOOT_AVREN_DDR|=_BV(BOOT_SD_AVR_EN);
#endif

            /* configure MOSI, SCK, SS lines as outputs */
#ifdef USE_SSS
            SS_DISABLE;
            SuskaSSSPI_DDR |= _BV(SuskaSPI_SS2) | _BV(SuskaSPI_SS1) |_BV(SuskaSPI_SS0) ;
#endif
            SuskaSPI_DDR |= _BV(SuskaSPI_MOSI) | _BV(SuskaSPI_SCK); 
}

#ifdef SD_IMAGEFILE
// Used for SD-Access (sd-command)
uint8_t getsdreq_status(void)
{
 return (BOOT_REQ_PIN&_BV(BOOT_REQ));
}
uint8_t waitsdreq(void)
{
	uint8_t ret=1;
        while(!getsdreq_status())
	{
		if(buttons_poll())
		{
			ret=0;
			break;
		}
#ifdef SHELL_POLL
// SHELL_POLL might use suska-spi
// do not Poll if SPI is already active
		if ((SuskaSSSPI_PORT & (_BV(SuskaSPI_SS2)|_BV(SuskaSPI_SS1)|_BV(SuskaSPI_SS0)))==(_BV(SuskaSPI_SS2)|_BV(SuskaSPI_SS1)|_BV(SuskaSPI_SS0))) 
		{
			SHELL_POLL();
		}
#endif

	}
	return ret;
}
#endif

#ifdef SUSKAFLASH
// Used for Bootflash (f-* commands)
uint8_t waitbreq(void)
{
	uint8_t ret=1;
        while(!(BOOT_REQ_PIN&_BV(BOOT_REQ)))
	{
		if(buttons_poll())
		{
			ret=0;
			break;
		}

	}
	return ret;
}

void sendack(void)
{

        // Generate ACK
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        BOOT_ACK_PORT|=_BV(BOOT_ACK);
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        BOOT_ACK_PORT&=~_BV(BOOT_ACK);
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
}

uint16_t sendfb(uint16_t v)
{
        uint16_t ret;

        ret=Suskaspi_send16(v);
        sendack();

        return ret;
}
#endif // SUSKAFLASH

uint8_t Suskaspi_send(uint8_t data)
{
                uint8_t rdata=0;

                for (uint8_t i=0; i<8; i++)
                {       if (data&128)
                        {
                                SuskaSPI_PORT|=_BV(SuskaSPI_MOSI);
                        }
                        else
                        {
                                SuskaSPI_PORT&=~_BV(SuskaSPI_MOSI);
                        }
                        data<<=1;
                        SuskaSPI_PORT|=_BV(SuskaSPI_SCK);
                        rdata<<=1;
                        if (SuskaSPI_PIN&(1<<SuskaSPI_MISO))
                        {
                                rdata|=1;
                        }
//                        SuskaSPI_PORT|=_BV(SuskaSPI_SCK);
#ifdef SUSKASPI_SLOW
                        asm("nop");
                        asm("nop");
                        asm("nop");
                        asm("nop");
#endif
                        SuskaSPI_PORT&=~_BV(SuskaSPI_SCK);
#ifdef SUSKASPI_SLOW
                        asm("nop");
                        asm("nop");
                        asm("nop");
                        asm("nop");
#endif
                }
                return rdata;
}


uint16_t Suskaspi_send16(uint16_t data)
{
                uint16_t rdata=0;

                for (uint8_t i=0; i<16; i++)
                {       if (data&0x8000)
                        {
                                SuskaSPI_PORT|=_BV(SuskaSPI_MOSI);
                        }
                        else
                        {
                                SuskaSPI_PORT&=~_BV(SuskaSPI_MOSI);
                        }
                        data<<=1;
                        SuskaSPI_PORT|=_BV(SuskaSPI_SCK);
                        rdata<<=1;
                        if (SuskaSPI_PIN&(1<<SuskaSPI_MISO))
                        {
                                rdata|=1;
                        }

//                        SuskaSPI_PORT|=_BV(SuskaSPI_SCK);
#ifdef SUSKASPI_SLOW
                        asm("nop");
                        asm("nop");
                        asm("nop");
                        asm("nop");
#endif
                        SuskaSPI_PORT&=~_BV(SuskaSPI_SCK);
#ifdef SUSKASPI_SLOW
                        asm("nop");
                        asm("nop");
                        asm("nop");
                        asm("nop");
#endif
                }
                return rdata;
}

#if defined OSD
void Suskaspi_send_n(unsigned char value, unsigned short cnt)
{
  while(cnt--)
    Suskaspi_send(value);
}
#endif

#if defined SUSKA_C | defined SUSKA_B | defined SUSKA_BF

uint32_t readFpgaVersion(uint16_t *type)
{
        static uint16_t t;
        uint8_t b;
        uint32_t ver=0;
        SS_ENABLEVER;
        for(uint8_t i=0;i<2;i++)
        {
                b=Suskaspi_send(0xff);
                t=(t<<8)|b;
        }
        *type=t;
        SS_DISABLE;
        SS_ENABLEVER;

        for(uint8_t i=0;i<4;i++)
        {
                b=Suskaspi_send(0xff);
                ver=(ver<<8)|b;
        }

        SS_DISABLE;
        return ver;
}
#endif
