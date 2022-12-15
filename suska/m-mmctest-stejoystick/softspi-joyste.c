/*
; Extended Joystick Port: Computer Side
;    _______________________________
;    \   5   4    3    2    1      /
;     \     10   9    8    7   6  /
;      \ 15  14   13  12   11    /
;       \_______________________/
;
; 4 - MOSI
; 3 - SCK
; 2 - CS
; 6 - MISO
; 7 - VCC
; 9 - GND
JOY_ADRS_OUTPUT		equ	$FF9202
JOY_SPI_CS_HIGH		equ	%1111111111111100
JOY_SPI_CS_LOW		equ	%1111111111111000
JOY_SPI_SCK_HIGH	equ	%0000000000000010
JOY_SPI_SCK_LOW		equ	%0000000000000000
JOY_SPI_DATA_1		equ	%0000000000000001	; MOSI must be bit 0 for the routine to work
JOY_SPI_DATA_0		equ	%0000000000000000
JOY_ADRS_INPUT_b	equ	$FF9201
JOY_SPI_MISO_BIT	equ	0

*/
#include "softspi.h"

#define JOY_OUTPUT      0xFF9202
#define JOY_INPUT       0xFF9201
#define SOFTSPI_CS      2 
#define SOFTSPI_SCK     1 
#define SOFTSPI_MOSI    0 
#define SOFTSPI_MISO    0 


uint8_t spiout;
uint8_t slowspi = 0;

volatile uint16_t* joy_output;
volatile uint8_t* joy_input;

void Softspi_init(void)
{
  joy_input=(uint8_t*)JOY_INPUT;
  joy_output=(uint16_t*)JOY_OUTPUT;
  spiout=0;
}

void Softspi_cs(uint8_t active)
{
  if(active)
  {
	spiout&=~_BV(SOFTSPI_MOSI);
  }
  else
  {
	spiout|=_BV(SOFTSPI_MOSI);
  }
  *joy_output = spiout; 
}

uint8_t noinline Softspi_send(uint8_t data)
{	
	uint8_t rdata=0;

        __asm__ __volatile__(
        "move.w #0x2700,sr\n\t"
        );

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
                *joy_output = spiout; 
	        if ((*joy_input)&(1<<SOFTSPI_MISO)) 
		{
			rdata|=1;
		}
		spiout|=_BV(SOFTSPI_SCK);
                *joy_output = spiout; 
		spiout&=~_BV(SOFTSPI_SCK);
                *joy_output = spiout; 
		data<<=1;
	}

        __asm__ __volatile__(
        "move.w #0x2300,sr\n\t"
        );

	return rdata;
}

