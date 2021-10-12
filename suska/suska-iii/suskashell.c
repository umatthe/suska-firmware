
/////////////////////////////////////
//                                 //
// Kommandozeilen-Parser           //
//                                 //
// Udo Matthe   05.01.2015         //
//                                 //
/////////////////////////////////////

#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "config.h"

#include "../timer/tick.h"
#include "../spi/spi.h"
#include "../mmc/mmc.h"
#include "../tff/ff.h"
#include "../misc/itoa.h"
#include "../uart-irq/uart-irq.h"
#include "power.h"

#ifndef SUSKA_C
#include "joystick.h"
#include "osd.h"
#endif

#include "../adc/adc.h"
#include "../coretype/coretypes.h"

#ifdef USE_SUSKASPI
#include "suskaspi.h"
#endif
#ifdef USB_LOOP
#include "../usb/usbloop.h"
#endif
#if defined( SUSKA_B)| defined( SUSKA_BF )
#include "../suska-iii/ps2int.h"
#include "mouse.h"
#include "keyboard.h"
#endif

extern long boot_id;
extern short boot_app_version;

extern uint8_t kb_available;
extern uint8_t ms_available;


extern FATFS fs;
extern uint32_t tracelevel;

uint32_t fpgaversion;
uint16_t fpgatype;

static uint16_t swap( uint16_t in)
{
        return (in<<8|((in>>8)&0xff));
}

static void fpgainfo(void)
{
#ifdef USE_SUSKASPI
          fpgaversion=readFpgaVersion(&fpgatype);
          if(fpgatype==(fpgaversion>>16)&0xffff)
          {
            fpgatype=0xaaaa;
          }
          coretype=(fpgatype>>8)&0xff;
          coresubtype=fpgatype&0xff;
#else
// does not work without suskaspi
#endif
}


#ifdef USB_GATE
uint32_t readMaxVersion(void)
{
	uint8_t b;
	uint32_t ver=0;
	SS_ENABLEMAX;

	for(uint8_t i=0;i<4;i++)
	{
		b=Suskaspi_send(0xff);
		ver=(ver<<8)|b;
	}

	SS_DISABLE;
	return ver;
}
#endif

static char val[10];
void shell_info( void)
{
	     uint32_t mversion;
	     uint32_t hversion;

             BOOTAVR_ENABLE;
             fpgainfo();
             BOOTAVR_DISABLE;
#if defined SUSKA_B | defined SUSKA_BF
	     uint16_t rawvoltage;
	     float voltage;
           
        adc_startConversion();
        while(!adc_ConvReady());
        rawvoltage=adc_GetValue();

        uart_puts_P("raw Input Voltage: ");
        uart_puthexword(rawvoltage);
        uart_eol();
	voltage=((rawvoltage/FACTOR)*(R1+R2))/(R2);
	sprintf(val,"%1.2fV\n\r",voltage);
        uart_puts_P("Input Voltage: ");
        uart_puts(val);
#elif defined SUSKA_C
        uart_puts_P("BOOTAVR: ");
        if( BOOT_AVREN_PORT&_BV(BOOT_SD_AVR_EN))
        {
           uart_puts_P("SDC or FBOOT");
        }
        else
        {
           uart_puts_P("SDC only");
        }
        uart_eol();
#endif

#ifdef USB_GATE
             mversion=readMaxVersion();
#endif

             uart_puts_P("ATMEGA-Version: "); uart_puthexlong(SWVERSION); uart_eol();
             printf("Boot image id: %08lx\n",boot_id);
             printf("Boot App Version: %04x\n",boot_app_version);

             uart_puts_P("FPGA-Coretype: "); 
             uart_puthexbyte(coretype); 
             uart_puts_P(" "); 
             uart_puthexbyte(coresubtype);
             uart_puts_P(" ("); 
             switch(coretype)
             {
#ifdef SUSKA_BF
               case CT_SUSKA_BF_TT:
                    uart_puts_P(BOARDTYPE);
                    uart_puts_P(" TT ");
                    show_suska_subtype();
                    break;
               case CT_SUSKA_BF_FALCON:
                    uart_puts_P(BOARDTYPE);
                    uart_puts_P(" FALCON ");
                    show_suska_subtype();
                    break;
               case CT_SUSKA_BF_STE:
                    uart_puts_P(BOARDTYPE);
                    uart_puts_P(" STE ");
                    show_suska_subtype();
                    break;
#endif
#ifdef SUSKA_C
               case CT_SUSKA_C_STE:
                    uart_puts_P(BOARDTYPE);
                    uart_puts_P(" STE ");
                    show_suska_subtype();
                    break;
               case CT_SUSKA_C_FALCON:
                    uart_puts_P(BOARDTYPE);
                    uart_puts_P(" FALCON ");
                    show_suska_subtype();
                    break;
#endif
#ifdef SUSKA_B
               case CT_SUSKA_B_STE:
                    uart_puts_P(BOARDTYPE);
                    uart_puts_P(" STE ");
                    show_suska_subtype();
                    break;
#endif
               case CT_ARCADE:
                 uart_puts_P("Arcade ");
                 switch(coresubtype)
                 {
                   case ST_INVADERS:
                     uart_puts_P("Space Invaders");
                     break;
                 default:
                     uart_puts_P("unknown");
                     break;
                 }
                 break;
               
               case CT_TEST:
                 uart_puts_P("TestCore ");
                 switch(coresubtype)
                 {
                   case ST_AVR:
                     uart_puts_P("AVR");
                     break;
                 default:
                     uart_puts_P("unknown");
                     break;
                 }
                 break;


               case CT_OLD:
                 uart_puts_P("Old Core without type");
                 break;

               case CT_MULTICOMP:
                 uart_puts_P("Multicomp Z80");
                 break;

               default:
                 uart_puts_P("Unknown Core");
                 break;
             }
             uart_puts_P(") "); uart_eol();

             uart_puts_P("FPGA-Version: "); uart_puthexlong(fpgaversion); uart_eol();

#ifdef USB_GATE
             uart_puts_P("MAX-Version: "); uart_puthexlong(mversion); uart_eol();
	     
#endif
}


#ifdef SUSKAFLASH
void shell_fdump( uint8_t *o, uint8_t *l)
{


        uint32_t offset;
        uint32_t len;
        uint8_t b;
        uint16_t data;

        BOOTAVR_ENABLE;
        power_fboot();

        sscanf(o,"%ld",&offset);
        sscanf(l,"%ld",&len);
        SS_ENABLEFLASHBOOT;
        waitbreq();
        printf("\nAddr: %x\n",sendfb(offset));

        waitbreq();
        printf("CMD: %x\n",sendfb(0x0017));
        printf("-- %08lx %08lx\n",(offset<<8),len);
        printf("FLASH Dump\n");

        for(uint32_t i=0; i<len;i++)
        {
                if(!(i%8)) printf("\n%08lx: ",(offset<<8)+i);
                data=sendfb(0xaffe);
                printf("%04x ",data);
        }
        SS_DISABLE;
        printf("\n");
}
void shell_fread( uint8_t *o, uint8_t *l, uint8_t *n)
{

        FIL handle;
        FRESULT res;
        uint16_t bw;

        uint32_t offset;
        uint32_t len;
        uint8_t b;
        uint16_t data;
        uint16_t atad;


        sscanf(o,"%ld",&offset);
        sscanf(l,"%ld",&len);

        f_mount(0, &fs);
        res = f_open(&handle, n, FA_CREATE_ALWAYS | FA_WRITE);

        BOOTAVR_ENABLE;
        SS_ENABLEFLASHBOOT;
        power_fboot();

        waitbreq();
        sendfb(offset);

        waitbreq();
        sendfb(0x0017);
        printf("\n-- %08lx %08lx %s\n",(offset<<8),len,n);
        printf("FLASH Read\n");

        for(uint32_t i=0; i<len;i++)
        {
                data=sendfb(0xaffe);
                atad=swap(data);
                res = f_write(&handle, &atad, sizeof(atad), &bw);
                if(!(i%(512L<<3)))
                {
                    uart_puts_P("TF-reading  0x");
                    uart_puthexword(i>>9);
                    uart_puts_P(" KB\r");
                }

        }
        printf("\nReading done\n");
        SS_DISABLE;
        f_close(&handle);
}

void shell_fwrite( uint8_t *o, uint8_t *n)
{
        FIL handle;
        FRESULT fres;
        uint16_t flen;
        uint16_t p;

        uint32_t offset;
        uint8_t  b;
        uint16_t ver=0;
        uint16_t data;
        uint16_t atad;
        uint32_t count=0;


        sscanf(o,"%ld",&offset);

        f_mount(0, &fs);
        fres = f_open(&handle, n, FA_READ);
        if(fres!=FR_OK)
        {
         printf("fopen failed\n");
        }
        else
        {
        BOOTAVR_ENABLE;
        SS_ENABLEFLASHBOOT;
        power_fboot();

        waitbreq();
        sendfb(offset);

        waitbreq();
        sendfb(0x0020);
        printf("\n-- %08lx %s\n",(offset<<8),n);
        printf("FLASH Write\n");

        while(1)
        {
                f_read(&handle, &atad, sizeof(atad), &flen);
                if(flen!=2) break;
                data=swap(atad);
                sendfb(data);
                count++;

                if(!(count%(512L<<3)))
                {
                    uart_puts_P("TF-writing  0x");
                    uart_puthexword(count>>9);
                    uart_puts_P(" KB\r");
                }

        }
        SS_DISABLE;
        f_close(&handle);
        printf("\nFLASH Writen (%ld Words)\n",count);
        }
}
void shell_ferase( void )
{


        BOOTAVR_ENABLE;
        SS_ENABLEFLASHBOOT;

        power_fboot();

        waitbreq();
        sendfb(0x0000);

        waitbreq();
        sendfb(0x0010);

        printf("\nErase Flash while LED is blinking ...\n");

        SS_DISABLE;
}
#endif

#ifdef SD_IMAGEFILE
#define f_size(fp)  ((fp)->fsize)
void shell_sd(uint8_t *n)
{
	uint32_t req;
	uint8_t b;
	uint16_t index;
        uint32_t block;
        uint8_t cmd;
        FIL handle;
        FRESULT fres;
        uint16_t flen;
        uint32_t fsize;


        f_mount(0, &fs);
        fres = f_open(&handle, n, FA_READ|FA_WRITE);
        if(fres!=FR_OK)
        {
         printf("fopen %s failed (%d)\n",n,fres);
        }
        else
        {
                fsize = f_size(&handle);
                printf("F-Size: %ld\n",fsize);
                BOOTAVR_ENABLE;
		while(1)
		{
			uart_puts_P("wait ... ");
			waitsdreq();
			uart_puts_P("REQ\n\r");

			SS_ENABLESD;
			for(uint8_t i=0;i<4;i++)
			{
				b=Suskaspi_send(0xAA);
				req=(req<<8)|b;
			}
			block=req&0x00ffffff;
			cmd  =(req&0x7f000000)>>24;
                        if(cmd==2) 
                        { 
                          uart_puts_P("\n\rexit Cmd\n\r");
                          sendack(); 
                          break; 
                        }
                        if((block<<9)>fsize)
                        { 
                          uart_puts_P("\n\rillegal req: ");
                          uart_puthexlong(req);
                          uart_eol();
                          sendack(); 
                          break; 
                        } 
			fres=f_lseek(&handle,block<<9);
                        if(fres!=FR_OK)
			{
                          uart_puts_P("\n\rseek failed\n\r");
                          break;
			}

			if(cmd==0) //read
			{
				//	uart_puts_P("cpm-sdc-request: "); uart_puthexlong(req); uart_eol();
				//	uart_puts_P("cmd: "); uart_puthexbyte(cmd); uart_eol();
				uart_puts_P("r: "); uart_puthexlong(block); 
				sendack();
				//	uart_puts_P("\n\rSend ACK\n\r");
        	
				for(index=0;index<512;index++)
				{

					f_read(&handle, &b, sizeof(b), &flen);
					waitsdreq();
					Suskaspi_send(b);
					sendack();
				}
				//	uart_puts_P("Send ACK ");uart_puthexword(index); uart_eol();

			}
			else //write
			{
				uart_puts_P("w: "); uart_puthexlong(block);
				sendack();
				for(index=0;index<512;index++)
				{
					waitsdreq();
					b=Suskaspi_send(0xee);
//					if(!(index%16)) {uart_eol(); uart_puthexword(index); uart_puts_P(" ");}
//				        uart_puthexbyte(b); uart_puts_P(" ");
					f_write(&handle, &b, sizeof(b), &flen);
					sendack();
				}
				uart_eol();	
				f_sync(&handle);
			}
			SS_DISABLE;
			uart_puts_P(" done\n\r");

		}
		SS_DISABLE;
                BOOTAVR_DISABLE;
		f_close(&handle);
	}
}
#endif

#if defined SUSKA_BF | defined SUSKA_B
void shell_ps2(void)
{
        ps2_scan();
}
#endif
#if defined SUSKA_BF 
void shell_joy( uint8_t *value)
{
	uint8_t i;
	uint8_t v;
	sscanf(value,"%d",&v);
        for(i=0;i<10;i++)
	joystick_sendspi(v+i);
//	joystick_sendspi(v);
}
#ifdef OSD
static char text[40];
void shell_osd( uint8_t *cmd)
{
	static uint8_t c=0;
        fpgainfo();
        uint16_t rawvoltage;
        float voltage;

        c=1-c;
        if(c)
        {
          OSD_SHOW();
          OsdSetTitle("Suska-BF");
          OsdWrite(0,"System Info" , 0);
          OsdWrite(1," " , 0);
          adc_startConversion();
          while(!adc_ConvReady());
          rawvoltage=adc_GetValue();
          voltage=((rawvoltage/FACTOR)*(R1+R2))/(R2);
          sprintf(text,"Input Voltage: %1.2fV",voltage);
          OsdWrite(2,text , 0);
          OsdWrite(3," " , 0);
          sprintf(text,"ATMEGA-Version: %lx ",SWVERSION);
          OsdWrite(4,text , 0);
          OsdWrite(5," " , 0);
#ifdef USE_SUSKASPI
          sprintf(text,"FPGA-Type: %x",fpgatype); 
          OsdWrite(6,text , 0);
          sprintf(text,"FPGA-Version: %lx",fpgaversion); 
          OsdWrite(7,text , 0);
#else
          OsdWrite(6," " , 0);
          OsdWrite(7," " , 0);
#endif
        }
        else
        {
          OSD_HIDE();
        }
}
#endif

#endif
#ifdef SPIRAM
void shell_rwrite( uint8_t *o, uint8_t *n)
{
        FIL handle;
        FRESULT fres;
        uint16_t flen;
        uint16_t p;

        uint32_t offset;
        uint8_t  b;
        uint16_t ver=0;
        uint16_t data;
        uint16_t atad;
        uint32_t count=0;


        sscanf(o,"%ld",&offset);

        f_mount(0, &fs);
        fres = f_open(&handle, n, FA_READ);
        if(fres!=FR_OK)
        {
         printf("fopen failed\n");
        }
        else
        {
        BOOTAVR_ENABLE;
        printf("RAM Write\n");

        while(1)
        {
                f_read(&handle, &atad, sizeof(atad), &flen);
                if(flen!=2) break;
                SS_ENABLERAMADDR;
                sendfb((offset+count)>>16);
                sendfb((offset+count)&0xFFFF);
                SS_DISABLE;
                SS_ENABLERAMDATA;
                data=swap(atad);
                sendfb(data);
                SS_DISABLE;
                count++;

                if(!(count%(512L<<3)))
                {
                    uart_puts_P("FR-writing  0x");
                    uart_puthexword(count>>9);
                    uart_puts_P(" KB\r");
                }

        }
        f_close(&handle);
        printf("\nRAM Written (%ld Words)\n",count);
        }
}

#endif
