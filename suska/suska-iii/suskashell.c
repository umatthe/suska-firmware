
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

#ifdef __HAVE_FILESYSTEM__
#include "../mmc/mmc.h"
#endif
#include "../timer/tick.h"
#include "../spi/spi.h"
#include "../tff/ff.h"
#include "../misc/itoa.h"
#include "../uart-irq/uart-irq.h"
#include "power.h"

#ifndef SUSKA_C
#include "joystick.h"
#include "osd.h"
#endif

#if defined SUSKA_B | defined SUSKA_BF
#include "../adc/adc.h"
#endif

#include "../coretype/coretypes.h"

#ifdef USE_SUSKASPI
#include "suskaspi.h"
#else
#define BOOTAVR_ENABLE
#define BOOTAVR_DISABLE
#endif
#ifdef USB_LOOP
#include "../usb/usbloop.h"
#endif
#if defined( SUSKA_B)| defined( SUSKA_BF )
#include "../suska-iii/ps2int.h"
#include "mouse.h"
#include "keyboard.h"
#endif
#if defined(SD_IMAGEFILE)
#include "../suska-iii/sdrawfile.h"
#endif

extern long boot_id;
extern short boot_app_version;

extern uint8_t kb_available;
extern uint8_t ms_available;


extern FATFS fs;
extern uint32_t tracelevel;
extern uint8_t sd_active;
extern uint8_t sd_verbose;

uint32_t fpgaversion;
uint16_t fpgatype;

static uint16_t swap( uint16_t in)
{
        return (in<<8|((in>>8)&0xff));
}

void fpgainfo(void)
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
#if defined SD_IMAGEFILE
             if(!sd_active)
#endif
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
//UMA needs float support	sprintf(val,"%1.2fV\n\r",voltage);
	sprintf(val,"%1d.%02dV\n\r",(int)voltage,(int)(voltage*100)-(int)voltage*100);
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
#if defined SUSKA_B | defined SUSKA_BF | SUSKA_C
             printf("Boot image id: %08lx\n",boot_id);
             printf("Boot App Version: %04x\n",boot_app_version);
#endif
#ifndef SUSKA_C_SYSCTRL
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
               case CT_SUSKA_IV_B_FALCON:
                    uart_puts_P(BOARDTYPE);
                    uart_puts_P(" FALCON ");
                    show_suska_subtype();
                    break;
               case CT_SUSKA_BF_STE:
               case CT_SUSKA_IV_B_STE:
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
#endif
#ifdef USB_GATE
             uart_puts_P("MAX-Version: "); uart_puthexlong(mversion); uart_eol();
	     
#endif // ifndef SUSKA_C_SYSCTRL
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
//UMA        printf("-- %08lx %08lx\n",(offset<<8),len);
        printf("-- %08lx %08lx\n",(offset),len);
        printf("FLASH Dump\n");

        for(uint32_t i=0; i<len;i++)
        {
                if(!(i%8))
                {
                       uart_eol();
                       uart_puthexlong((offset<<9)+(i<<1)); //UMA 8
                       uart_puts_P(" : ");
                }

                data=sendfb(0xaffe);
                uart_puthexword(data);
                uart_puts_P(" ");
        }
        SS_DISABLE;
        uart_eol();
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
void shell_ferase( uint8_t *base )
{
        uint32_t start;
        BOOTAVR_ENABLE;
        if(strncmp(base,"all",3) == 0)
        {


        SS_ENABLEFLASHBOOT;
        power_fboot();
        waitbreq();
        sendfb(0x0000);
        waitbreq();
        sendfb(0x0010);
        printf("\nErase Flash while LED is blinking ...\n");
        SS_DISABLE;
        }
        else
        {
               sscanf(base,"%ld",&start);
               printf("\nErase Flash 512K Slot: %d ",start);
#ifdef SUSKA_C
               if(start !=0)
               {
                 printf(" out of range (use 0, select Slot with dipswitches)\n");
               }
               
#else
               if(start > 15)
               {
                 printf(" out of range (0..15)\n");
               }
#endif
               else
               {
                if (tracelevel < 3)
                {
                   printf("\n disabled (use trace 3)\n");
                }
                else
                {
                  SS_ENABLEFLASHBOOT;
                  power_fboot();
                  for(uint8_t i=0;i<4;i++)
                  {
                    waitbreq();
                    printf("\nAddr: %x\n",sendfb((start<<2)+i));
                    waitbreq();
                    sendfb(0x0012);
                  }
                  SS_DISABLE;
                  uart_eol();
                }
              }
       }
}
#endif

#ifdef SD_IMAGEFILE
void shell_sd(uint8_t *n, uint8_t *t)
{
        BOOTAVR_ENABLE;

        switch(t[0])
        {
        case 'o':
        {
          sdraw_openfile(n,0);

	  while(1) // Blocking loop shell is blocked.
	  {
		uart_puts_P("wait ... ");
		waitsdreq();
		uart_puts_P("REQ\n\r");

                if(sdraw_handle_req()) break;

	  }
          sdraw_closefile();
          BOOTAVR_DISABLE;
         }
         break;
         case 'i':
         {
           uart_puts_P("SD Info\n");
           printf("Status: %s\n",sd_active?"Active":"Inactive");
           printf("Debug: %s\n",sd_verbose?"Active":"Inactive");
         }
         break;
         case 'a':
         {
           sdraw_openfile(n,1);
         }
         break;
         case 'A':
         {
           sdraw_openfile(n,2);
         }
         break;
         case 'd':
         {
           sdraw_closefile();
           BOOTAVR_DISABLE;
           uart_puts_P("SD-File disabled\n");
         }
         break;
         default:
         {
                printf("unknown Option: %s\n",t);
                uart_puts_P("a - activate file\n");
                uart_puts_P("d - deactivate file\n");
                uart_puts_P("o - open file blocking\n");
                uart_puts_P("i - info\n");
         }
         break;
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
#ifdef avrcore
void shell_kb(void)
{
    uart_puts_P("Atari-Keyboard\n");
}
#endif
