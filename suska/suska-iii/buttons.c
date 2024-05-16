#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "power.h"
#include "../shell/parser.h"

#ifdef SUSKA_C
#error "No Buttons at Suska-C SDC"
#endif

void buttons_init(void)
{
}

uint8_t buttons_poll( void )
{
 uint8_t res=0;
 static uint32_t po=0;
 static uint8_t rb=0;
 static uint32_t sb=0;
 static uint8_t cb=0;
 static uint8_t rstatus=0;
 static uint8_t cstatus=0;
 static uint8_t secondreset=0;
 if(!rb)
 {
#ifdef RESET_BUTTON_PINS
   if((RESET_BUTTON_PINS & _BV(POWER_RESET_BUTTON)) != 0)
#else
   if((POWER_RESET_BUTTON_PIN & _BV(POWER_RESET_BUTTON)) != 0)
#endif
   {
	   if(!rstatus)
	   {	
		   if(secondreset)
		   {
			   res=1;
		   }
		   set_reset_pin(1); 
		   rstatus=1;
		   secondreset=1;
	   }
	   rb=100;
   }
   else
   {
	   po=0;
	   if(rstatus)
	   {
		   set_reset_pin(0);
		   rstatus=0;
		   sb=20000;
	   }
	   else
	   {
		   if(sb) sb--;
		   if(!sb)
		   {
			   secondreset=0;
		   }
	   }
   }
 }
 else
 {
	 rb--;
	 po++;
	 if(po>100000) 
	 {
		 switch_power(0);  // FPGA Power off
#ifdef RESET_BUTTON_PINS
		 while((RESET_BUTTON_PINS & _BV(POWER_RESET_BUTTON)) != 0); // Wait for Button release
#else
		 while((POWER_RESET_BUTTON_PIN & _BV(POWER_RESET_BUTTON)) != 0); // Wait for Button release
#endif
		 shell_reset(); // --> Restart AVR
		 // never reached
	 }
 }

 if(!cb)
 {
#ifdef RESET_BUTTON_PINS
	 if((RESET_BUTTON_PINS & _BV(CORE_RESET_BUTTON)) != 0)
#else
	 if((CORE_RESET_BUTTON_PIN & _BV(CORE_RESET_BUTTON)) != 0)
#endif
	 {
		 if(!cstatus)
		 {	
			 set_reset_core_pin(1);
			 cstatus=1;
		 }
		 cb=100;
	 }
	 else
		 if(cstatus)
		 { 
			 set_reset_core_pin(0);
			 cstatus=0;
		 }
 }
 else
 {
	 cb--;
 }
 return res;
}
