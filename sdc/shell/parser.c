/////////////////////////////////////
//                                 //
// Kommandozeilen-Parser           //
//                                 //
// Udo Matthe   08.12.2012         //
//                                 //
/////////////////////////////////////


#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "parser.h"

//#include "../timer/tick.h"
#include "../spi/spi.h"
#include "../mmc/mmc.h"
#include "../tff/ff.h"
#include "../misc/itoa.h"
#include "../uart-irq/uart-irq.h"

#include "commands.h"
#include "parser.h"

#include "shelldefs.h"

extern uint8_t fsline[];
extern uint32_t tracelevel;

extern struct timedate uhr;

struct befehle befehl;
uint8_t *parameters[MAXPARAMETER];
uint8_t comtxt[MAXCMDLEN+1];
uint8_t helptxt[MAXHELPLEN+1];
uint8_t parse_line(uint8_t *line)
{
	uint8_t parameter;
	uint8_t pos;

	for(uint8_t i=0;i<MAXCOMMAND;i++)
	{
		memcpy_P(&befehl,&befehle[i],sizeof(befehl));

		if(!befehl.command) return 1; // Tabellen-Ende - Befehl nicht gefunden
		strncpy_P((char*)&comtxt,(befehl.command),MAXCMDLEN);
		strncpy_P((char*)&helptxt,(befehl.help),MAXHELPLEN);
		if(strncmp(line,comtxt,strlen(comtxt)) == 0)
		{
			pos=0;
			parameter=befehl.parameter;
			if(tracelevel>1)
			{
				uart_puts_P("Command: ");
				uart_puts(comtxt);
				uart_puts_P(" ");
			}
			uint8_t *paras=strtok(line," ");
			while(parameter>0)
			{
				parameter--;
				if(tracelevel>1) uart_puts_P("Parameter: ");
				// Parameter filename
				paras=strtok(NULL," ");
				if(paras!=0)
				{
					parameters[pos]=paras;
					if(tracelevel>1)
					{
						uart_puts(paras);
						uart_puts_P(" ");
					}
					pos++;
				}
				else
				{
					uart_puts_P("missing parameter: ");
			                uart_puts(comtxt);
                                        uart_puts_P(" -- ");
					uart_puts(helptxt);
					uart_eol();
					return 1;
				}
			}
			if(tracelevel>1) uart_eol();
			
			switch(befehl.parameter)
			{
				case 0:
					(*(void(*)(void)) (befehl.function))();
					break;
				case 1:
					if(tracelevel>0)
					{
						uart_puts_P("Parameter: ");
						uart_puts(parameters[0]);
						uart_eol();
					}
					(*(void(*)(uint8_t*)) (befehl.function))(parameters[0]);
					break;
				case 2:
					if(tracelevel>0)
					{
						uart_puts_P("Parameter: ");
						uart_puts(parameters[0]);
						uart_puts_P(" * ");
						uart_puts(parameters[1]);
						uart_eol();
					}
					(*(void(*)(uint8_t*, uint8_t*)) (befehl.function))(parameters[0],parameters[1]);
					break;
				case 3:
					if(tracelevel>0)
					{
						uart_puts_P("Parameter: ");
						uart_puts(parameters[0]);
						uart_puts_P(" * ");
						uart_puts(parameters[1]);
						uart_puts_P(" * ");
						uart_puts(parameters[2]);
						uart_eol();
					}
					(*(void(*)(uint8_t*, uint8_t*, uint8_t*)) (befehl.function))(parameters[0],parameters[1],parameters[2]);
					break;
				default:
					break;
			}

			return 0;
		}
	}
	return 1;
}

void shell_reset( void )
{
	MMC_Off();
	asm("jmp 0xF800");
}

void shell_trace( uint8_t *para )
{
	uint32_t level;
	sscanf(para,"%li",&level);
	tracelevel=level;
}

void shell_delay( uint8_t *para )
{
	uint32_t deltime;
	sscanf(para,"%li",&deltime);
        for(uint32_t i=0;i<(deltime>>6);i++)
	{
		//_delay_ms(deltime);
		_delay_ms(64);
	}	
}

#ifdef __HAVE_FILESYSTEM__
void shell_ls( void )
{
	show_dir(0);	
}

void shell_do( uint8_t *paras )
{
	parse_file(paras);
}

void shell_hex( uint8_t *paras )
{
	if(tracelevel>1) uart_puts_P("CMD: hex\r\n");
	if(tracelevel>0)
	{
		uart_puts("Para filename: ");
		uart_puts(paras);
		uart_eol();
	}
	hexdump_file(paras,0);
}

void shell_cat( uint8_t *paras )
{
	if(tracelevel>1) uart_puts_P("CMD: cat\r\n");
	if(tracelevel>0)
	{
		uart_puts("Para filename: ");
		uart_puts(paras);
		uart_eol();
	}
	show_file(paras,0);
}
#endif

void shell_help( void )
{
	uart_puts_P("** YH-FPGA-Shell **\r\n");
	uart_eol();
	for(uint8_t i=0;i<MAXCOMMAND;i++)
	{
		memcpy_P(&befehl,&befehle[i],sizeof(befehl));
		if(befehl.command)
		{
                        strncpy_P((char*)&comtxt,(befehl.command),MAXCMDLEN);
                        strncpy_P((char*)&helptxt,(befehl.help),MAXHELPLEN);
			uart_puts(comtxt);
                        uart_puts_P(" -- ");
			uart_puts(helptxt);
                        uart_eol();
		}
		else
		{
			uart_puts_P("------------\n\r");
			break;
		}
	}
}


#ifdef __HAVE_FILESYSTEM__
FRESULT parse_file(char *filename)
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
				// Ausfuehren
				if(tracelevel>0)
				{
					uart_puts_P("<");
					uart_puts(fsline);
					uart_puts_P(">");
					uart_eol();
				}
				parse_line(fsline);

                } 

        }
        f_close(&handle);
	return res;
}
#endif
