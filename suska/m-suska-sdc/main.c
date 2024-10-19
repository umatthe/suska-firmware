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

#include "config.h"
#include "../suska-iii/power.h"
#include "../shell/shell.h"
#include "../shell/parser.h"
#include "../timer/tick.h"
#include "../suska-iii/suskaspi.h"
#include "../uart-irq/uart-irq.h"
#include "../asisp/asisp.h"
#include "../asisp/asappl.h"
#include "../suska-iii/suskashell.h"
#include "../coretype/coretypes.h"
#include "../adc/adc.h"

#if defined (SUSKA_BF) | defined (SUSKA_B)
#include "../suska-iii/joystick.h"
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

#ifdef SUSKA_B
#include <avr/wdt.h>
//void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
__attribute__((naked,section(".init3")))
void wdt_init(void)
{
        cli();
        wdt_reset();
        MCUSR &= ~(1<<WDRF);
        WDTCSR |= (1<<WDCE) | (1 << WDE);
        WDTCSR = 0;
}
#endif

int main(void)
{
#if defined( SUSKA_BF ) | defined( SUSKA_B )
        tick_init(TICK10MS);
#else
        tick_init(TICK100MS);
#endif
#ifdef SHELL_HAVE_LED 
        led_init();
        // 3 mal Blinken zum Start 
        for(uint8_t i=0;i<3;i++)
        {
                led_on();
                _delay_ms(500);
                led_off();
                _delay_ms(500);
        }
#else 
#ifdef SHELL_HAVE_UART_LED
        led_init();
        // Suska-BF-Pico Board uses this Pin to switch the SCC-RS232/Debug-UART 
        // led_on()  == High == RS232
        // led_off() == Low  == Uart
        // todo: Add command to change setting from fpga-shell.
        led_on();
#endif
#endif

#ifdef USE_SUSKASPI
        Suskaspi_init();
#endif
#ifndef SUSKA_C_SYSCTRL
        BOOT_ACK_PORT&=~_BV(BOOT_ACK);
        BOOT_ACK_DDR|=_BV(BOOT_ACK);
#endif

	uart_init(UART_UBRR);
	uart_irqinit();
        uart_puts_P("** UART-Init done **\r\n");
#ifdef  EN_PS
        power_init();
        uart_puts_P("** Power init done (power off) **\r\n");
#else
        power_resetpin_init();
        uart_puts_P("** Button Init done **\r\n");
#endif

#ifndef SUSKA_C_SYSCTRL
        memcpy_P(&current_bootldrinfo, (uint8_t*) FLASHEND - INFOBOOTLDRSIZE - sizeof(bootldrinfo_t) + 1, sizeof(bootldrinfo_t));
        boot_id=current_bootldrinfo.dev_id;
        boot_app_version=current_bootldrinfo.app_version;
#endif
#ifdef EN_PS // CTL has Power Switch

// Wait for Power Button pressed
        int key;
#ifdef RESET_BUTTON_PINS
	while ((RESET_BUTTON_PINS & _BV(POWER_RESET_BUTTON)) == 0)
#else
	while ((POWER_RESET_BUTTON_PIN & _BV(POWER_RESET_BUTTON)) == 0)
#endif
        {
           key=uart_getc_nowait();
           delayms(1000);
           if(key=='#') { break; }
        }
	switch_power(1);
#if defined SUSKA_BF
	adc_init(ADCREF_INT256);     //ADCREF_INT256 == Internal Ref 1.1V on 649 / 2.56V on 644
#endif
#if defined SUSKA_B
	adc_init(ADCREF_ARef);     //ADCREF_AREF == connected to VCC=3.3V on SUSKA_B
#endif
#ifndef SUSKA_C_SYSCTRL
	adc_setChannel(ADC_CHANNEL);
	adc_startConversion();
	while(!adc_ConvReady());
#endif
        uart_puts_P("** Power on done **\r\n");

#ifndef SUSKA_C_SYSCTRL
        uart_puts_P("Raw Voltage: ");
        uart_puthexword( adc_GetValue());
	uart_eol();
#endif
        // Suska System Reset
        delayms(50);
        set_reset_core_pin(1);
        delayms(50); 
        set_reset_core_pin(0);
        delayms(500); 
        set_reset_pin(1);
        delayms(50); 
        set_reset_pin(0);
        delayms(500); 
#if defined SUSKA_B
        // Activate reset to allow AVR-Access to SD-Card and SPI-Devs
        set_reset_pin(1);
        delayms(50); /* wait until stable condition */
#endif
#endif // EN_PS // CTL has Power Switch
        uart_puts_P("** Init done **\r\n");
        stdout = &mystdout;
        shell_info();
        if (coretype == CT_TEST)
        {
           set_reset_core_pin(0);
           delayms(100);
           set_reset_core_pin(1);
           uart_eol();
        }

#if defined SUSKA_BF | defined SUSKA_B
	timer_init();
	ps2_init();
#endif
#if defined SUSKA_BF 
        joystick_init();
#endif
#if defined SUSKA_BF | defined SUSKA_B | defined SUSKA_C
#ifdef HAVE_EE_AS_ENABLE
	as_init(true);
        powerdownas( eeprom_read_byte(&ee_asactive));
#endif
	as_init(false);
#endif
	while(1)
	{
		shell_init();
#if defined SUSKA_BF | defined SUSKA_B
                uart_eol();
                ps2_scan();
#endif
#ifndef NO_SHELL
		shell_loop();
#else
                // Check for # Key to enable Shell on B-Board 
                for(uint8_t wtime=0;wtime<3;wtime++) 
                {
                  uart_puts_P("** press # to activate shell **\r\n");
                  key=uart_getc_nowait();
                  delayms(1000); 
                  if(key=='#') { shell_loop(); }
                }
                
                // Suska-B normal operation mode
                // disable AVR-SPI/MMC and enable joystick
                uart_puts_P("** No Shell active **\r\n");
                SoftSPI_DDR &= ~(_BV(SoftSPI_MOSI) | _BV(SoftSPI_SCK));
                _MMC_DDR_ &= ~_BV(_MMC_Chip_Select_);
                SuskaSSSPI_DDR &= ~(_BV(SuskaSPI_SS2) | _BV(SuskaSPI_SS1) |_BV(SuskaSPI_SS0)) ;
                SuskaSPI_DDR &= ~(_BV(SuskaSPI_MOSI) | _BV(SuskaSPI_SCK));
                SS_ENABLESD;  // All SSx Pins 0 - disable Pullup
                joystick_init();
#ifdef JOYDEBUGLOOP
                uint8_t debugtimeout=60;
                while(debugtimeout)
                {
                  joystick_poll();
                  uart_puts_P("Joystick1 Value: ");
                  uart_puthexbyte(joystick_poll());
                  uart_eol();
                  _delay_ms(1000);
                  debugtimeout--;
                }
#endif
                // Trigger Reset
                set_reset_pin(1); delayms(50); 
                set_reset_pin(0); delayms(500); 
                // Set UART to ATARI-Keyboard-Uart-Speed
                // (a Terminal connected to the Console will show garbage)
                uart_init(ATARIKEYBUBRR);
                // Loop to do the ATARI Core PS2/Joystick stuff
                while(1)
                {
#ifdef SHELL_POLL
                        SHELL_POLL();
#else
#warning "nix zu tun"
#endif
                }
#endif
	}
}
