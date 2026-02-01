/////////////////////////////////////
//                                 //
// ASISP YellowHut FPGA-Loader     //
//                                 //
//                                 //
// Udo Matthe   02.12.2012         //
//                                 //
/////////////////////////////////////

#include "../shell/shell.h"
#include "../shell/commands.h"

#include "config.h"
#include "../timer/tick.h"
#include "asisp.h"
#include "asappl.h"

static uint32_t int_start;
static uint32_t int_len;

void powerdownas( uint8_t en )
{
        if(!en) //!=0
        {
	  uart_puts_P("AS-Disabled");
          uart_eol();
	  as_cso_lo();
	  as_write(POWERDOWN);
	  as_cso_hi();
        }
}

void printasstatus( void )
{
	uint8_t x=as_readstatus();
	uart_puts_P("AS-Status: ");
	uart_puthexbyte(x);
	uart_puts_P(" : ");
	for(int8_t i=7;i>=0;i--)
	{
		if(x&_BV(i))
		{
			uart_puts_P("1");
		}
		else
		{
			uart_puts_P("0");
		}
	}
	uart_eol();
}


uint8_t getasid( void )
{
	int8_t val;

	as_init(true);
	as_cso_lo();
	as_write(SILICONID);
	as_write(0);
	as_write(0);
	as_write(0);
	val=as_read();
	as_cso_hi();
        delayms(1);
        return val;
}

uint32_t getaslen( void )
{
	int8_t val;
	uint32_t len;

	switch(getasid())
	{
		case 0x10:
			len=128L*1024L;
		break;
		
		case 0x12:
			len=512L*1024L;
		break;
		
		case 0x14:
			len=2048L*1024L;
		break;
                
		case 0x15:
                       len=4096L*1024L;
                break;
		
		case 0x16:
			len=8192L*1024L;
		break;

		case 0x17:
			len=16384L*1024L;
		break;

                //UMA Hack FF for Micron Assume 8M
                case 0xff:
			len=16384L*1024L;
		break;

		default:
			len=0;
		break;
	}
	return len;

}

uint32_t checkaslen( void )
{
        uint8_t p;
	uint32_t count=0;
	uint32_t len=0;
	uint32_t fsize;
                
                fsize=getaslen();

                as_init(true);
                as_cso_lo();
                as_write(READ);
                as_write(0);
                as_write(0);
                as_write(0);
                len=0;
                for(uint32_t l=0;l<fsize;l++)
                {
                        p=as_read();
                        //UMA dont care FF==FF if(endian==0) p=mirror(p); //LowBit first
                        if (p != 0xff)
                        {
                          len=count;
                        } 
                        count++;
                        if(!(count%(512L<<3)))
                        {
                                uart_puts_P("F-checking  0x");
                                uart_puthexword(count>>10);
                                uart_puts_P(" KB\r");
                        }
                }
                as_cso_hi();
                as_init(false);
                uart_eol();

        return len;
}

uint8_t mirror(uint8_t in)
{
	uint8_t out=0;
	for(uint8_t i=0;i<8;i++)
	{
		if(in&_BV(i))
		{
			out=(out<<1)|1;
		}
		else
		{
			out=(out<<1);
		}
	}
	return out;
}

void flashassave(uint8_t *fname,uint32_t fsize, uint32_t ctaddr,uint8_t endian)
{


        FIL handle;
        FRESULT res;
        uint16_t len;
	uint8_t p;
	uint32_t count=0;

	uart_puts(fname);
	uart_puts_P(" ");
	uart_puthexlong(int_start);
	uart_puts_P(" ");
	uart_puthexlong(int_len);
	uart_eol();

        res = f_open(&handle, fname,  FA_WRITE|FA_CREATE_ALWAYS);
        if(res!=FR_OK)
        {
                // Error Handling
		uart_puts_P("Error: Unable to create: ");
		fserr_out(res);
        }
        else
        {
		as_init(true);
		as_cso_lo();
		as_write(READ);
		as_write(0);
		as_write(0);
		as_write(0);
		len=0;	
		for(uint32_t l=0;l<fsize;l++)
		{
			p=as_read();
			if(endian==0) p=mirror(p); //LowBit first
			f_write(&handle, &p, 1, &len);
			count++;
			if(!(count%(512L<<3)))
			{
				uart_puts_P("F-reading  0x");
				uart_puthexword(count>>10);
				uart_puts_P(" KB\r");
			}
		}
		as_cso_hi();
		as_init(false);
		uart_puts_P("\n\rdone ...\n\r");
	}
	f_close(&handle);
	uart_puts_P("File ");
	uart_puts(fname);
	uart_puts_P(" created.\n\r");
}

void flashaswrite(uint8_t *fname, uint32_t ctaddr,uint8_t endian)
{


        FIL handle;
        FRESULT res;
        uint16_t len;
	uint8_t p;
	uint32_t count=0;
	uint8_t led=0;

	uart_puts(fname);
	uart_puts_P(" ");
	uart_puthexlong(int_start);
	uart_eol();

        res = f_open(&handle, fname,  FA_READ);
        if(res!=FR_OK)
        {
                // Error Handling
		uart_puts_P("Error: Unable to read: ");
		fserr_out(res);
        }
        else
        {
		as_init(true);
		uart_puts_P("FLash: erasing ... ");
		led_off();
		if(as_erase())
                {
		  uart_puts_P("done\n\r");
                }
                else
                {
		  uart_puts_P("error\n\r");
                }
		len=0;	

                while(1)
		{
			f_read(&handle, &p, 1, &len);
			if(len!=1) break;
			if(endian==0) p=mirror(p); //LowBit first
			if(!(count%256))
			{
				as_write256start(count);
			}
			as_write(p);
			count++;
			if(!(count%(512L<<3)))
			{
				uart_puts_P("F-writing  0x");
				uart_puthexword(count>>10);
				uart_puts_P(" KB\r");
			}
			if(!(count%(512L<<3)))
			{
				if(led)
				{
					led_on();
				}
				else
				{
					led_off();
				}
				led=1-led;
			}
		}
		as_write256done();
		as_init(false);
		uart_puts_P(" done ...\n\r");

		for(uint8_t i=0;i<50;i++)
		{
			led_on();
			_delay_ms(20);
			led_off();
			_delay_ms(20);
		}
	}
	f_close(&handle);
	uart_puts_P("File ");
	uart_puts(fname);
	uart_puts_P(" flashed.\n\r");
}

