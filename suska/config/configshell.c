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

void sendconfig(void)
{
 config_sendspi(eeprom_read_byte(&ee_config1), eeprom_read_byte(&ee_config2));
}


void shell_setconfig(uint8_t *para)
{
       uint16_t config;
       sscanf(para,"%d",&config);
       eeprom_write_byte(&ee_config1,(config&0xff00)>>8);
       eeprom_write_byte(&ee_config2,config&0x00ff);
       sendconfig();

}
void shell_getconfig(void)
{
                uart_puts_P("FPGA-Config: ");
                uart_puthexbyte(eeprom_read_byte(&ee_config1));
                uart_puthexbyte(eeprom_read_byte(&ee_config2));
                uart_eol();
}


