/////////////////////////////////////
//                                 //
// Main File YellowHut FPGA-Loader //
//                                 //
// Für Suska-FPGA Board o.ae.      //
//                                 //
// Udo Matthe   02.12.2012         //
//                                 //
/////////////////////////////////////

#include "commands.h"

static  FILINFO finfo;

uint8_t fsline[LINELEN];
uint32_t tracelevel = DEBUGTRACELEVEL;

static char *p;
char *strtok(char *s, const char *delim)
{
	    return strtok_r(s, delim, &p);
} 

FRESULT show_dir(int8_t channel)
{
	DIR dir;
	uint32_t  p1;
        FRESULT res;
	uint16_t s1, s2;
	uint8_t *fn;
#ifdef _USE_LFN  // defined in ff.h
        uint8_t Lfname[_MAX_LFN];
#endif

		res = f_opendir(&dir, "/");
		if (res==FR_OK)
		{
		p1 = s1 = s2 = 0;

#ifdef _USE_LFN  // defined in ff.h
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
#ifdef _USE_LFN  // defined in ff.h
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
			  uart_puts(fsline);
			  uart_eol();
			}
		}
		sprintf(fsline,"%4u File(s),%10lu bytes total\r\n%4u Dir(s)", s1, p1, s2);
		if(channel==0)
		{
			uart_puts(fsline);
			uart_eol();
		}
		}
		return res;

}

FRESULT hexdump_file(char *filename, uint8_t channel)
{
        FIL handle;
        FRESULT res;
        uint16_t len;
	char c;

        res = f_open(&handle, filename,  FA_READ);
        if(res!=FR_OK)
        {
                // Error Handling
		uart_puts_P("Error: Unable to open: ");
		fserr_out(res);
        }
        else
        {
		// Aus dem File lesen
		uint32_t addr=0;

                while(1)
		{
			f_read(&handle, &c, 1, &len);
			if(len==0) break;

			if(channel==0)
			{
				if(!(addr%16))
				{
					uart_eol();
					uart_puthexlong(addr);
					uart_puts_P(" : ");
				}
				uart_puthexbyte(c);
				uart_puts_P(" ");

			}
			addr++;
                } 

        }
	uart_eol();
        f_close(&handle);
	return res;
}




FRESULT show_file(char *filename, uint8_t channel)
{
        FIL handle;
        FRESULT res;

        res = f_open(&handle, filename,  FA_READ);
        if(res!=FR_OK)
        {
                // Error Handling
		uart_puts_P("Error: Unable to open: ");
		fserr_out(res);
        }
        else
        {
		// Aus dem File lesen
                 while(readline(fsline,80,&handle))
                {
			if(channel==0)
			{
					uart_puts(fsline);
					uart_eol();
			}

                } 

        }
        f_close(&handle);
	return res;
}


/*-----------------------------------------------------------------------*/
/* Get a string from the file                                            */
/*-----------------------------------------------------------------------*/
uint8_t* readline ( uint8_t* buff, uint8_t len, FIL* fil )
{
	uint8_t i = 0;
	uint8_t *p = buff;
	uint16_t rc;


	while (i < len - 1) {                   /* Read bytes until buffer gets filled */
		f_read(fil, p, 1, &rc);
		if (rc != 1) break;                     /* Break when no data to read */
		if (*p == '\r') continue;       /* Strip '\r' */
		i++;
		if (*p == '\n') break;        /* Break when reached end of line */
		*p++;
	}
	*p = 0;
	return i ? buff : 0;                    /* When no data read (eof or error), return with error. */
}

void fserr_out(FRESULT res)
{
	uart_puts_P(" fres=");
	uart_puts(itoa(res));
	uart_eol();
}

