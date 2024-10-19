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
#ifdef __HAVE_FILESYSTEM__
#include "../mmc/mmc.h"
#include "../tff/ff.h"
#endif
#include "../misc/itoa.h"
#include "../asisp/asisp.h"
#include "../asisp/asappl.h"
#include "../asisp/asshell.h"
#include "../uart-irq/uart-irq.h"

extern uint32_t tracelevel;

	
#ifdef __HAVE_FILESYSTEM__
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
#endif

void shell_aspowerdown(void)
{
  
		as_init(true);
                as_cso_hi();
                delayms(1);
//                as_idle();
                powerdownas(0); 
                delayms(1);
		as_init(false);
}

void shell_asgetid( void)
{
		uint8_t val;
		uint32_t len;

		as_init(true);
#ifdef HAVE_EE_AS_ENABLE
		uart_puts_P("AS-Active: ");
                uart_puthexbyte(eeprom_read_byte(&ee_asactive));
		uart_eol();
#endif
		printasstatus();
                val=getasid();
		uart_puts_P("Silicon-ID: ");
		uart_puthexbyte(val);
		uart_puts_P(" ");

		switch(val)
		{
			case 0x10:
				uart_puts_P("EPCS1");
				break;
		
			case 0x12:
				uart_puts_P("EPCS4");
				break;

			case 0x14:
				uart_puts_P("EPCS16");
				break;
		
			case 0x15:
				uart_puts_P("EPCSQ32");
				break;

			case 0x16:
				uart_puts_P("EPCS64");
				break;

			default:
				uart_puts_P("** unknown **");
				break;
		}
		uart_eol();
		uart_puts_P("Size: 0x");
		uart_puthexlong(getaslen());
		uart_puts_P(" Byte");
		uart_eol();
		as_init(false);

}
#ifdef HAVE_EE_AS_ENABLE
void shell_asenable( uint8_t *para )
{
        uint8_t asen;
        sscanf(para,"%d",&asen);
        uart_puts_P("Enable ");
        uart_puthexbyte(asen);
        uart_eol();
        eeprom_write_byte(&ee_asactive, asen);
}
#endif
