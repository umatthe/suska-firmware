/////////////////////////////////////
//                                 //
// Config Speicher                 //
//                                 //
// Udo Matthe   05.12.2025         //
//                                 //
/////////////////////////////////////

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "configshell.h"
#include "../misc/itoa.h"
#include "../uart-irq/uart-irq.h"
#include "../suska-iii/suskaspi.h"

uint8_t ee_config1 EEMEM;
uint8_t ee_config2 EEMEM;
uint8_t ram_config1;
uint8_t ram_config2;


void initconfig(void)
{

 ram_config1=eeprom_read_byte(&ee_config1);
 ram_config2=eeprom_read_byte(&ee_config2);
 config_sendspi(ram_config1, ram_config2);
}

void sendramconfig(void)
{
 config_sendspi(ram_config1, ram_config2);
}


void shell_setconfig(uint8_t *para)
{
       uint16_t config;
       sscanf(para,"%d",&config);
       //eeprom_write_byte(&ee_config1,(config&0xff00)>>8);
       //eeprom_write_byte(&ee_config2,config&0x00ff);
       ram_config1=((config&0xff00)>>8);
       ram_config2=(config&0x00ff);
       sendramconfig();

}


void shell_cartconfig(uint8_t *para)
{
       uint8_t slot;
       sscanf(para,"%d",&slot);
       if((slot < 16)||(slot>31))
       {
                uart_puts_P("Out of Range 16..31\n\r");
       }
       else
       {
        slot-=16;
        ram_config2=(ram_config2&0xc3)|(slot<<2);
        sendramconfig();
       }

}

void shell_bitconfig(uint8_t *bit, uint8_t *val)
{
    uint8_t b;
    uint8_t v;
    sscanf(bit,"%d",&b);
    if(b<8)
    {
      if (val[0]=='1')
         ram_config2|=_BV(b);
      else
         ram_config2&=~_BV(b);
    }
    else if(b<16)
    {
      b-=8;
      if (val[0]=='1')
         ram_config1|=_BV(b);
      else
         ram_config1&=~_BV(b);
    }
    
    sendramconfig();
}

void shell_getconfig(void)
{
                uart_puts_P("FPGA-Config    RAM: ");
                uart_puthexbyte(ram_config1);
                uart_puthexbyte(ram_config2);
                uart_puts_P(" : ");
                uart_putbinbyte(ram_config1);
                uart_putbinbyte(ram_config2);
                uart_puts_P("\n\rCart: ");
                uart_putdecbyte(16+((ram_config2>>2)&0x0f));
                uart_eol();
                if(ram_config1 != eeprom_read_byte(&ee_config1) |
                   ram_config2 != eeprom_read_byte(&ee_config2))
                {
                   uart_puts_P("FPGA-Config EEPROM: ");
                   uart_puthexbyte(eeprom_read_byte(&ee_config1));
                   uart_puthexbyte(eeprom_read_byte(&ee_config2));
                   uart_puts_P(" : ");
                   uart_putbinbyte(eeprom_read_byte(&ee_config1));
                   uart_putbinbyte(eeprom_read_byte(&ee_config2));
                   uart_eol();
                   uart_puts_P("use c-save to uptdate EEPROM\n\r");
                }
                else
                {
                   uart_puts_P("Config EEPROM ok\n\r");
                }
}


void shell_saveconfig(uint8_t *para)
{
        uint8_t force = 0;
        if ((ram_config2&HW_CONFIG_MASK) != 
            (eeprom_read_byte(&ee_config2)&HW_CONFIG_MASK))
        {
          force = 1;
        }
        if(strncmp(para,"suska",5) == 0)
        {
          force = 0;
        }
        
        if(!force)
        {
         eeprom_write_byte(&ee_config1,ram_config1);
         eeprom_write_byte(&ee_config2,ram_config2);
         uart_puts_P("EEPROM updated.\n\r");
        }
        else
        {
           uart_puts_P("EEPROM not updated - wrong Mode\n\r");
        }
}
