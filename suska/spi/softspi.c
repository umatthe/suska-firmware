#include <avr/io.h>
#include "config.h"
#include "spi.h"
#include <util/delay.h>

uint8_t slowspi = 0;

void Softspi_init(void)
{

    /* configure MOSI, SCK lines as outputs */
#ifndef SoftSPI_SS
    SoftSPI_DDR |= _BV(SoftSPI_MOSI) | _BV(SoftSPI_SCK);
#else
    SoftSPI_DDR |= _BV(SoftSPI_MOSI) | _BV(SoftSPI_SCK) | _BV(SoftSPI_SS);
#endif
}

void Softspi_z(void)
{
    /* configure MOSI, SCK lines as inputs */
#ifndef SoftSPI_SS
    SoftSPI_DDR &= ~(_BV(SoftSPI_MOSI) | _BV(SoftSPI_SCK));
#else
    SoftSPI_DDR &= ~(_BV(SoftSPI_MOSI) | _BV(SoftSPI_SCK) | _BV(SoftSPI_SS));
#endif
}


uint8_t noinline Softspi_send(uint8_t data)
{	
	uint8_t rdata=0;

	for (uint8_t i=0; i<8; i++)
	{	if (data&128)
		{
			SoftSPI_PORT|=_BV(SoftSPI_MOSI);
		}
		else
		{
			SoftSPI_PORT&=~_BV(SoftSPI_MOSI);
		}
		rdata<<=1;
	        if (SoftSPI_PIN&(1<<SoftSPI_MISO)) 
		{
			rdata|=1;
		}
                if(slowspi)
                {
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   SoftSPI_PORT|=_BV(SoftSPI_SCK);
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   SoftSPI_PORT&=~_BV(SoftSPI_SCK);
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
		   asm("nop");
                }
                else
                {
		   SoftSPI_PORT|=_BV(SoftSPI_SCK);
		   SoftSPI_PORT&=~_BV(SoftSPI_SCK);
                }
	        data<<=1;
	}
	return rdata;
}


#ifdef SPI_USE_CHUNK
void SoftSpiWriteChunk( const uint8_t* pChunk, uint16_t nBytes )
{
        while (nBytes--)
        {
		Softspi_send( *pChunk++ );
        }
}

void SoftSpiReadChunk( uint8_t* pChunk, uint16_t nBytes )
{
        while (nBytes--)
        {
                *pChunk++ = Softspi_send(0);
        }

}
#endif
