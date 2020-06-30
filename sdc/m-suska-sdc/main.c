/////////////////////////////////////
//                                 //
// Main File YellowHut FPGA-Loader //
//                                 //
// Für Suska-FPGA Board o.ae.      //
//                                 //
// Udo Matthe   02.12.2012         //
//                                 //
/////////////////////////////////////

#include "bootloader.h"

#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
//#include <avr/pgmspace.h>

#include "config.h"
#include "../suska-iii/power.h"
#include "../shell/shell.h"
#include "../shell/parser.h"
#include "../timer/tick.h"
#include "../suska-iii/suskaspi.h"
#include "../uart-irq/uart-irq.h"
#include "../asisp/asisp.h"
#include "../suska-iii/suskashell.h"

#include "../adc/adc.h"

#if defined( SUSKA_B )| defined (SUSKA_BF)
#include "../suska-iii/joystick.h"
#endif
#if defined( SUSKA_BF )
#include "../suska-iii/ps2int.h"
#include "../suska-iii/mouse.h"
#include "../suska-iii/keyboard.h"
#include "../suska-iii/timer_overflow.h"
extern uint8_t kb_available;
extern uint8_t ms_available;
#endif
static bootldrinfo_t current_bootldrinfo;
long boot_id;
short boot_app_version;


extern FILE mystdout;


int main(void)
{
#if defined( SUSKA_BF )
        tick_init(TICK10MS);
#else
        tick_init(TICK100MS);
#endif
#ifdef SHELL_LED
        led_init();
        // 3 mal Blinken zum Start 
        for(uint8_t i=0;i<3;i++)
        {
                led_on();
                _delay_ms(500);
                led_off();
                _delay_ms(500);
        }
#endif 

#ifdef USE_SUSKASPI
        Suskaspi_init();
#endif
        BOOT_ACK_PORT&=~_BV(BOOT_ACK);
        BOOT_ACK_DDR|=_BV(BOOT_ACK);

//#ifdef SUSKA_C
//        BOOT_TRIGGER_PORT&=~_BV(BOOT_TRIGGER);
//        BOOT_TRIGGER_DDR|=_BV(BOOT_TRIGGER);
//#endif
	uart_init();
	uart_irqinit();
        uart_puts_P("** UART-Init done **\r\n");
#ifdef  EN_PS
        power_init();
        uart_puts_P("** Power init done (power off) **\r\n");
#else
        power_resetpin_init();
        uart_puts_P("** Button Init done **\r\n");
#endif

        memcpy_P(&current_bootldrinfo, (uint8_t*) FLASHEND - INFOBOOTLDRSIZE - sizeof(bootldrinfo_t) + 1, sizeof(bootldrinfo_t));
        boot_id=current_bootldrinfo.dev_id;
        boot_app_version=current_bootldrinfo.app_version;
#ifdef EN_PS // CTL has Power Switch

// Wait for Power Button pressed
        int key;
	while ((RESET_BUTTON_PINS & _BV(POWER_RESET_BUTTON)) == 0)
        {
           key=uart_getc_nowait();
           delayms(1000);
           if(key=='#') { break; }
        }

	switch_power(1);

	adc_init(ADCREF_INT256);     //ADCREF_INT256 == Internal Ref 1.1V on 649 / 2.56V on 644
	adc_setChannel(ADC_CHANNEL);
	adc_startConversion();
	while(!adc_ConvReady());

        uart_puts_P("** Power on done **\r\n");

        uart_puts_P("Raw Voltage: ");
        uart_puthexword( adc_GetValue());
	uart_eol();
        delayms(50); /* wait until stable condition */
        set_reset_core_pin(1);
        delayms(50); /* wait until stable condition */
        set_reset_core_pin(0);
        delayms(500); /* wait until stable condition */
        set_reset_pin(1);
        delayms(50); /* wait until stable condition */
        set_reset_pin(0);
        delayms(500); /* wait until stable condition */
#endif
        uart_puts_P("** Init done **\r\n");
        shell_info();

#ifdef SUSKA_BF
	timer_init();
	ps2_init();
#endif
#ifndef SUSKA_C
	joystick_init();
#endif
	as_init(false);
        stdout = &mystdout;


	while(1)
	{
		shell_init();
#ifdef SUSKA_BF
		ps2_dev_init();
		uart_puts_P("\n\rKeyboard: ");
		uart_puthexbyte(kb_available);
		uart_puts_P("\n\rMouse: ");
		uart_puthexbyte(ms_available);
		uart_eol();
#endif
		shell_loop();
	}
}
