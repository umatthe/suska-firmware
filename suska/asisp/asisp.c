/////////////////////////////////////
//                                 //
// ASISP YellowHut FPGA-Loader     //
//                                 //
//                                 //
// Udo Matthe   02.12.2012         //
//                                 //
/////////////////////////////////////

#include "asisp.h"

#ifdef HAVE_EE_AS_ENABLE
#include <avr/eeprom.h>
uint8_t ee_asactive EEMEM;
#endif

uint8_t as_confdone( void )
{
		if(AS_PIN&AS_CONFDONE)
			return 1;
		else
			return 0;
}

uint8_t as_init( bool access )
{
	if(access)
	{
	   AS_PORT|=AS_DCLK|AS_CSO|AS_DO|AS_FPGACE;
#if (defined SUSKA_BF) | (defined SUSKA_B) 
	   AS_PORT&=~(AS_CONFIG);
	   AS_DDR=AS_DCLK|AS_CSO|AS_DO|AS_FPGACE|AS_CONFIG;
#else
	   AS_DDR=AS_DCLK|AS_CSO|AS_DO|AS_FPGACE;
#endif
	}
	else
	{
		AS_PORT&=~(AS_FPGACE); 
#if (defined SUSKA_BF) | (defined SUSKA_B) 
	        AS_PORT|= (AS_CONFIG);
		AS_DDR&=~(AS_DCLK|AS_CSO|AS_DO|AS_FPGACE|AS_CONFIG);
#else
		AS_DDR&=~(AS_DCLK|AS_CSO|AS_DO|AS_FPGACE);
#endif
	}
	_delay_ms(10);
	return (as_confdone());
}

void as_clock( void )
{
	as_dclk_lo();
	as_dclk_hi();
}

void as_write( uint8_t x )
{
	for(int8_t i=7;i>=0;i--)
	{
		if(x&_BV(i))
		{
			as_do_hi();
		}
		else
		{
			as_do_lo();
		}
		as_clock();
	}
}

uint8_t as_read( void )
{
	uint8_t val=0;
	for(int8_t i=7;i>=0;i--)
	{
		as_clock();
		if(AS_PIN&AS_DATA)
		{
			val|=_BV(i);
		}
	}
	return val;
}

void as_writeenable( void )
{
	as_cso_lo();
	as_write(WRITEENABLE);
	as_cso_hi();
}

void as_writedisable( void )
{
	as_cso_lo();
	as_write(WRITEDISABLE);
	as_cso_hi();
}

void as_writestatus( uint8_t x )
{
	as_cso_lo();
	as_write(WRITESTATUS);
	as_write(x);
	as_cso_hi();
}

uint8_t as_readstatus( void )
{
	uint8_t val;
	as_cso_lo();
	as_write(STATUS);
	val=as_read();
	as_cso_hi();
	return val;
}

uint8_t as_idle( void )
{
	uint16_t retry = IDLETIMEOUT;
	uint8_t s=0;
	do
	{
		if(retry == 0) break;
		retry--;
		s=as_readstatus();
		_delay_ms(10);
		
	}
	while(s!=0);
	if(retry)
		return 1;
	else
		return 0;
}

void as_erase( void )
{
	uint8_t val;
	as_idle();
	as_writeenable();
	as_cso_lo();
	as_write(ERASEBULK);
	as_cso_hi();
	as_idle();
}

void as_write256start(uint32_t addr)
{
	as_cso_hi();
	as_idle();
	as_writeenable();
	as_cso_lo();
	as_write(WRITE);
	as_write((addr>>16)&0xff);
	as_write((addr>>8)&0xff);
	as_write(addr&0xff);
}

void as_write256done( void )
{
	as_idle();
}
