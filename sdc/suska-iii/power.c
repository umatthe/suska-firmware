#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../uart-irq/uart-irq.h"
#include "../timer/tick.h"
#include "../mmc/mmc.h"

#include "power.h"

void power_resetpin_init(void)
{
#ifndef SUSKA_C
        RESET_CORE_PORT&=~_BV(RESET_CORE);   // Set to 0
        RESET_PORT&=~_BV(RESET);             // Set to 0
#else
        RESET_CORE_PORT|=_BV(RESET_CORE);   // Set to 1
        RESET_PORT|=_BV(RESET);             // Set to 1
#endif
        RESET_CORE_DDR|=_BV(RESET_CORE);     // Output
        RESET_DDR|=_BV(RESET);               // Output
}

void power_fboot( void )
{
#ifdef SUSKA_C
       uart_puts_P(" res");
       set_reset_pin(0);
       uart_puts_P("...");
        delayms(500);

       uart_puts_P(" cres");
       set_reset_core_pin(0);
       uart_puts_P("...");
        delayms(500);


       uart_puts_P(" cres");
       set_reset_core_pin(1);
        uart_puts_P("...");
        delayms(500);

       uart_puts_P(" res");
       set_reset_pin(1);
        uart_puts_P("...");
        delayms(500);

       uart_puts_P(" res");
       set_reset_pin(0);
        uart_puts_P("...");
        delayms(500);

       uart_puts_P(" res");
       set_reset_pin(1);
        uart_puts_P("...");
#else
       uart_puts_P(" res");
       set_reset_pin(1);
       uart_puts_P("...");
        delayms(500);

       uart_puts_P(" cres");
       set_reset_core_pin(1);
       uart_puts_P("...");
        delayms(500);


       uart_puts_P(" cres");
       set_reset_core_pin(0);
        uart_puts_P("...");
        delayms(500);

       uart_puts_P(" res");
       set_reset_pin(0);
        uart_puts_P("...");
        delayms(500);

       uart_puts_P(" res");
       set_reset_pin(1);
        uart_puts_P("...");
        delayms(500);

       uart_puts_P(" res");
       set_reset_pin(0);
        uart_puts_P("...");
#endif
}

/**
 * Set reset pin
 */
void set_reset_pin(uint8_t on)
{
           if (on)
	   {
             RESET_PORT|=_BV(RESET);   // Set to 1
             uart_puts_P(" r1 ");
	   }
           else
	   {
             RESET_PORT&=~_BV(RESET);   // Set to 0
	     uart_puts_P(" r0 ");
	   }
}

/**
 * Set reset core pin
 */
void set_reset_core_pin(uint8_t on)
{
           if (on)
	   {
             RESET_CORE_PORT|=_BV(RESET_CORE);   // Set to 1
             uart_puts_P(" c1 ");
	   }
	   else
	   {
             RESET_CORE_PORT&=~_BV(RESET_CORE);   // Set to 0
	     uart_puts_P(" c0 ");
	   }
}

void shell_res( uint8_t *level)
{
 uint8_t l;
 sscanf(level,"%d",&l);

 uart_puts_P("reset ");
 set_reset_pin(l);
 uart_puts_P("\r\n");
}

void shell_cres( uint8_t *level)
{
 uint8_t l;
 sscanf(level,"%d",&l);

 uart_puts_P("core reset ");
 set_reset_core_pin(l);
 uart_puts_P("\r\n");
}

#ifndef SUSKA_C

volatile uint8_t power_on = 0;

void power_init(void)
{
        power_resetpin_init();
        EN_PS_PORT&=~_BV(EN_PS);   // Set to 0 Power 0ff
        EN_PS_DDR|=_BV(EN_PS);     // Output
	mmc_hwinit();
}

/**
 * Switch power on/off
 */
uint8_t switch_power(uint8_t on)
{
	uint8_t result = 0;
	uint8_t waitloop = 10;

	if (power_on != on) 
	{
		if(on)
			EN_PS_PORT|=_BV(EN_PS);   // Set to 1 Power On
		else
			EN_PS_PORT&=~_BV(EN_PS);
		
		delayms(100);

		while ((!(FPGA_CONFIG_DONE_PIN&_BV(FPGA_CONFIG_DONE))) && on && waitloop)
		{
			delayms(300);
			uart_puts_P("wait conf ...\r\n");
			waitloop--;
		}
		if(!waitloop)
		{
			uart_puts_P("timeout ... FPGA load failed\r\n");
		}

		if (on) {
			result = 1;
		}
		power_on = on;
	}
	return result;
}



#endif // !SUSKA_C
