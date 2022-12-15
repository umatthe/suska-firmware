/*
; Printer Port: Computer Side
;    _____________________________________________
;    \  13 12 11 10  9  8  7  6  5   4  3  2  1  /
;     \  25 24 23 22 21 20 19 18 17 16 15 14    /
;      \_______________________________________/
;
;  2 - MOSI PortB0
;  4 - SCK  PortB2
;  --UMA 1 - CS   PortA5 (Strobe)
;  3 - CS   PortB1 
; 11 - MISO MFP0   (Busy)
; 13 - VCC
; 25 - GND
*/
#include "softspi.h"

#define AY_SELREG       0xFF8800
#define AY_WRITEREG     0xFF8802
#define CTRL_REG        7
#define PRTDAT_REG      15
#define PRT_INPUT       0xFFFA01
// Outputs
#define SOFTSPI_MOSI    0 
#define SOFTSPI_CS      1 
#define SOFTSPI_SCK     2 
// Input
#define SOFTSPI_MISO    0 

#include <stdio.h>

uint8_t slowspi = 0;
uint8_t spiout;

volatile uint8_t* aysel_output;
volatile uint8_t* aywrite_output;
volatile uint8_t* prt_input;

void Softspi_init(void)
{
  prt_input=(uint8_t*)PRT_INPUT;
  aysel_output=(uint8_t*)AY_SELREG;
  aywrite_output=(uint8_t*)AY_WRITEREG;

  for (uint8_t i=0;i<16;i++)
  {
   *aysel_output = i;
   printf("%d - %02x\n",i,*aysel_output);
  }

  *aysel_output = CTRL_REG;
  *aywrite_output= (*aysel_output) | 0x80; 
  spiout=0;
  Softspi_cs(0);
}

void Softspi_cs(uint8_t active)
{
  *aysel_output = PRTDAT_REG;
  if(active)
  {
	spiout&=~_BV(SOFTSPI_CS);
  }
  else
  {
	spiout|=_BV(SOFTSPI_CS);
  }
  *aywrite_output=spiout;
}

uint8_t noinline Softspi_send(uint8_t data)
{	
	uint8_t rdata=0;
        __asm__ __volatile__(
        "move.w #0x2700,sr\n\t"
        );

        *aysel_output = PRTDAT_REG;
	for (uint8_t i=0; i<8; i++)
	{	

		rdata<<=1;
                if (data&128)
		{
			spiout|=_BV(SOFTSPI_MOSI);
		}
		else
		{
			spiout&=~_BV(SOFTSPI_MOSI);
		}
                *aywrite_output=spiout;
	        if ((*prt_input)&(1<<SOFTSPI_MISO)) 
		{
			rdata|=1;
		}
		spiout|=_BV(SOFTSPI_SCK);
                *aywrite_output=spiout;
		spiout&=~_BV(SOFTSPI_SCK);
                *aywrite_output=spiout;
		data<<=1;
	}
        __asm__ __volatile__(
        "move.w #0x2300,sr\n\t"
        );
	return rdata;
}

