
/////////////////////////////////////
//                                 //
// Udo Matthe   02.04.2022         //
//                                 //
/////////////////////////////////////

#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "config.h"

#include "../tff/ff.h"
#include "../uart-irq/uart-irq.h"

#ifdef USE_SUSKASPI
#include "suskaspi.h"
#endif

extern FATFS fs;
extern uint32_t tracelevel;
uint8_t sd_active=0;
uint8_t sd_verbose=0;


#ifdef SD_IMAGEFILE
        FIL handle;
        uint32_t fsize;
#define f_size(fp)  ((fp)->fsize)

uint8_t sdraw_handle_req(void)
{
	uint32_t req;
	uint8_t b;
	uint16_t index;
        uint32_t block;
        uint8_t cmd;
        FRESULT fres;
        uint16_t flen;

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
                          sd_active=0;
                          return 1;

                        }
                        if((block<<9)>fsize)
                        {
                          uart_puts_P("\n\rillegal req: ");
                          uart_puthexlong(req);
                          uart_eol();
                          sendack();
                          sd_active=0;
                          SS_DISABLE;
                          return 1;
                        }
                        fres=f_lseek(&handle,block<<9);
                        if(fres!=FR_OK)
                        {
                          uart_puts_P("\n\rseek failed\n\r");
                          sd_active=0;
                          SS_DISABLE;
                          return 1;
                        }
                        switch (cmd)
                        {
                         case 0: //read
                         {
                             if(sd_verbose) {
                                uart_puts_P("r: "); uart_puthexlong(block);
                             }
                                sendack();

                                for(index=0;index<512;index++)
                                {
                                        f_read(&handle, &b, sizeof(b), &flen);
                                        waitsdreq();
                                        Suskaspi_send(b);
                                        sendack();
                                }
                         }
                         break;
                         case 1: //write
                         {
                             if(sd_verbose) {
                                uart_puts_P("w: "); uart_puthexlong(block);
                             }
                                sendack();
                                for(index=0;index<512;index++)
                                {
                                        waitsdreq();
                                        b=Suskaspi_send(0xee);
                                        f_write(&handle, &b, sizeof(b), &flen);
                                        sendack();
                                }
                                f_sync(&handle);
                         }
                         break;
                         default:
                          uart_puts_P("\n\runknown req: ");
                         break;
                        }
                        SS_DISABLE;
                        if(sd_verbose)  uart_puts_P(" done\n\r");
                        return 0;
}

uint32_t sdraw_openfile(uint8_t *n, uint8_t poll)
{
        FRESULT fres;

        f_mount(0, &fs);
        fres = f_open(&handle, n, FA_READ|FA_WRITE);
        if(fres!=FR_OK)
        {
         printf("fopen %s failed (%d)\n",n,fres);
         sd_active=0;
        }
        else
        {
         fsize = f_size(&handle);
         printf("F-Size: %ld\n",fsize);
         sd_active=poll;
         sd_verbose=(poll&2);
        }
        return fsize;
}

void sdraw_closefile(void)
{
        sd_active=0;
        sd_verbose=0;
	SS_DISABLE;
	f_close(&handle);
}

void sdraw_poll(void)
{
    if(sd_active)
    {
         if(getsdreq_status())
         {
            sdraw_handle_req();
         }
    }
}
#endif
