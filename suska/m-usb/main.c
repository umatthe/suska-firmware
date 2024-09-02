/////////////////////////////////////
//                                 //
// Main File YellowHut FPGA-Loader //
//                                 //
// Für Suska-FPGA Board o.ae.      //
//                                 //
// Udo Matthe   02.12.2012         //
//                                 //
/////////////////////////////////////

#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "../uart-irq/uart-irq.h"
#include "../timer/tick.h"
#include "../shell/shell.h"
#include "../shell/parser.h"
#include "../suska-iii/suskaspi.h"
#include "../suska-iii/suskashell.h"

extern FILE mystdout;

//void shell_info(void)
//{
//             uart_puts_P("ATMEGA-Version: "); uart_puthexlong(SWVERSION); uart_eol();
//}

int main(void)
{
        stdout = &mystdout;
        tick_init(TICK100MS);

	uart_init(UART_UBRR);
//	uart_irqinit();
        uart_puts_P("** UART-Init done **\r\n");

#ifdef USE_SUSKASPI
        Suskaspi_init();
#endif

        shell_init();
        shell_info();
        shell_loop();

//        DDRE=0xff;
//        while(1)
//        {
//         PORTE=(PORTE+1)&0xFF;
//        }
}
