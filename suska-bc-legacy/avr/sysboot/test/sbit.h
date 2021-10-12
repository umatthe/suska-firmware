/*
 * Usage: 
 *
 * Define a port pin (input, output or ddr) like:
 *
 * 	#define key_input	SBIT(PINB,  2)	// Input Port B pin 2
 * 	#define led_output	SBIT(PORTB,  3)	// Output Port B pin 3 
 *
 * and access the 
 *
 * 	if (key_input)
 *		led_output = 1;
 *
 */
#ifndef _sbit_h_
#define _sbit_h_

struct bits {
  uint8_t b0:1;
  uint8_t b1:1;
  uint8_t b2:1;
  uint8_t b3:1;
  uint8_t b4:1;
  uint8_t b5:1;
  uint8_t b6:1;
  uint8_t b7:1;
} __attribute__((__packed__));


#define SBIT(port, pin) ((*(volatile struct bits*)&port).b##pin)

#endif
