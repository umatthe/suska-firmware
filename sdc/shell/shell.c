/////////////////////////////////////
//                                 //
//                                 //
//                                 //
// Udo Matthe   08.12.2012         //
//                                 //
/////////////////////////////////////

#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>

#include "config.h"
#ifndef _SHELL_TITLE_ 
#define _SHELL_TITLE_ "*** AVR FPGA-Shell ***\r\n"
#endif
#ifdef __HAVE_FILESYSTEM__
#include "../mmc/mmc.h"
#include "../tff/ff.h"
FATFS fs;            // Work area (file system object) for logical drive
#endif
#include "../misc/itoa.h"
#include "../timer/tick.h"
#include "../spi/spi.h"
#include "../uart-irq/uart-irq.h"
#include "../shell/shell.h"
#include "../shell/parser.h"


void shell_init(void)
{
	led_init();
//	led_on();

#ifdef SIGNALTICK
	tick_init(TICK10MS);
#endif
#ifdef __HAVE_FILESYSTEM__
//	mmc_hwinit();
//        MMC_Off();
//        MMC_Disable();
//UMA        _MMC_DDR_ |= _BV(_MMC_Chip_Select_);
	_delay_ms(1000);
        spi_init();
//        SoftSPI_DDR |= _BV(PB6) | _BV(PB5);
//        SoftSPI_PORT |= _BV(PB6) | _BV(PB5);
#endif

//	uart_init();
	uart_irqinit();

#ifdef SHELL_LED
	// 3 mal Blinken zum Start
	for(uint8_t i=0;i<3;i++)
	{
		led_on();
		_delay_ms(200);
		led_off();
		_delay_ms(200);
	}
#endif

#ifdef __HAVE_FILESYSTEM__
	// /init.txt als Startup-Batch-Datei ausführen, falls vorhanden
	uint8_t filename[]="/init.txt";


//        MMC_On();
	f_mount(0, NULL);
	f_mount(0, &fs);
	if(parse_file(filename)!=FR_OK)
	{
		uart_puts_P("FPGA Default init-File (init.txt) failed\r\n");
	}
#endif
	_delay_ms(200);

	uart_puts_P(_SHELL_TITLE_);
	uart_puts_P("YH-fpga-shell:> ");
#ifdef SHELL_INIT
        SHELL_INIT();
#endif

}

void shell_loop(void)
{
	uint8_t llen;
	uint8_t line[UART_BUFSIZE_IN+1];
	while(1)
	{
		line[0]=0x00;

		llen=uart_getline(line,UART_BUFSIZE_IN);
		if((llen==0) & (line[0]==0x00)) 
		{
			// Keine Taste gedrueckt...
			led_off();
#ifdef SHELL_POLL
                        if (SHELL_POLL()) 
			{
				// DoppelReset -> Warmstart
				uart_puts_P("\n\rAVR-Restart\r\n");
				break;
			}
#endif 
			continue;
		}
		if(llen>0)
		{
		        led_on();
			// Cmd eingegeben....
			if(parse_line(line)!=0)
			{
				uart_puts_P("Error: ");
				uart_puts(line);
				uart_eol();
			}
		}
		// Cmd abgearbeitet....
		uart_puts_P("fpga-shell:> ");
		led_off();

	}
}

