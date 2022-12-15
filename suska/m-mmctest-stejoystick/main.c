
#include <stdio.h>
#include "softspi.h"
#include "mmc.h"
#include "osbind.h"
#include "../tff/ff.h"
FATFS fs;            // Work area (file system object) for logical drive
static  FILINFO finfo;
#define LINELEN 80
uint8_t fsline[LINELEN];

//uint32_t tracelevel=1;
uint32_t tracelevel=3;

static uint8_t buffer[512];

FRESULT show_dir(int8_t channel)
{
        DIR dir;
        uint32_t  p1;
        FRESULT res;
        uint16_t s1, s2;
        uint8_t *fn;
#if _USE_LFN != 0 // defined in ff.h
        uint8_t Lfname[_MAX_LFN];
#endif

                res = f_opendir(&dir, "/");
                if (res==FR_OK)
                {
                p1 = s1 = s2 = 0;

#if _USE_LFN != 0  // defined in ff.h
                finfo.lfname = Lfname;
                finfo.lfsize = sizeof(Lfname);
#endif

                for(;;)
                {
                        res = f_readdir(&dir, &finfo);
                        if ((res != FR_OK) || !finfo.fname[0]) break;
                        if (finfo.fattrib & AM_DIR) {
                                s2++;
                        } else {
                                s1++; p1 += finfo.fsize;
                        }
#if _USE_LFN != 0  // defined in ff.h
                        if(finfo.lfname[0]==0)
                        {
                                fn=finfo.fname;
                        }
                        else
                        {
                                fn=finfo.lfname;
                        }
#else
                        fn=finfo.fname;
#endif
                        sprintf(fsline,"%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s",
                                        (finfo.fattrib & AM_DIR) ? 'D' : '-',
                                        (finfo.fattrib & AM_RDO) ? 'R' : '-',
                                        (finfo.fattrib & AM_HID) ? 'H' : '-',
                                        (finfo.fattrib & AM_SYS) ? 'S' : '-',
                                        (finfo.fattrib & AM_ARC) ? 'A' : '-',
                                        (finfo.fdate >> 9) + 1980, (finfo.fdate >> 5) & 15, finfo.fdate & 31,
                                        (finfo.ftime >> 11), (finfo.ftime >> 5) & 63,
                                        finfo.fsize,
                                        fn
                               );
                        if(channel==0)
                        {
                          puts(fsline);
                        }
                }
                sprintf(fsline,"%4u File(s),%10lu bytes total\r\n%4u Dir(s)", s1, p1, s2);
                if(channel==0)
                {
                        puts(fsline);
                }
                }
                else
                { 
                        printf("show_dir / failed with res=%d\r\n",res);
                }

                return res;

}


int main(void)
{
  short us;
  uint8_t success;
  printf("SD Test\n");
  us=Super(0L);
  Softspi_init();
  success=mmc_init();
  Super(us);
  if(success)
  { 
    us=Super(0L);
    mmc_read_sector (0L ,buffer );
    mmc_dump_buffer(buffer,512);
    f_mount(0, NULL);
    f_mount(0, &fs);
        show_dir(0);
    Super(us);
  }

}
