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

#include "../timer/tick.h"
#include "../spi/spi.h"
#include "../mmc/mmc.h"
#include "../tff/ff.h"
#include "../misc/itoa.h"
#include "../asisp/asisp.h"
#include "../asisp/asappl.h"
#include "../asisp/asshell.h"
#include "../uart-irq/uart-irq.h"

extern uint32_t tracelevel;

	

void shell_asread(uint8_t *fname)
{

	uint32_t len;

	len=getaslen();
	if(tracelevel>1) 
	{
		uart_puts_P("Fixed Parameter: len, using len= ");
		uart_puthexlong(len);
		uart_eol();
	}
	flashassave(fname,len,0,0); // Start 0 / Endian 0
}

void shell_aswrite(uint8_t *fname)
{
	flashaswrite(fname,0,0); // Start 0 / Endian 0
}
	

void shell_asgetid( void)
{
		uint8_t val;
		uint32_t len;

		as_init(true);
		printasstatus();
		as_cso_lo();
		as_write(SILICONID);
		as_write(0);
		as_write(0);
		as_write(0);
		val=as_read();
		as_cso_hi();
		as_init(false);
		uart_puts_P("Silicon-ID: ");
		uart_puthexbyte(val);
		uart_puts_P(" ");

		switch(val)
		{
			case 0x10:
				len=128L*1024L;
				uart_puts_P("EPCS1");
				break;
		
			case 0x12:
				len=512L*1024L;
				uart_puts_P("EPCS4");
				break;

			case 0x14:
				len=2048L*1024L;
				uart_puts_P("EPCS16");
				break;
		
			case 0x15:
				len=4096L*1024L;
				uart_puts_P("EPCSQ32");
				break;

			case 0x16:
				len=8192L*1024L;
				uart_puts_P("EPCS64");
				break;

			default:
				len=0;
				uart_puts_P("** unknown **");
				break;
		}
		uart_eol();
		uart_puts_P("Size: 0x");
		uart_puthexlong(len);
		uart_puts_P(" Byte");
		uart_eol();

}
