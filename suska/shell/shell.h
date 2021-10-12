/////////////////////////////////////
//                                 //
//                                 //
//                                 //
// Udo Matthe   08.12.2012         //
//                                 //
/////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "config.h"

#ifdef SHELL_HAVE_LED
#ifdef SHELL_LED
#define led_init() (SHELL_LEDDDR|=SHELL_LED)
#ifdef SUSKA_BF
#define led_on()  (SHELL_LEDPORT|=SHELL_LED)
#define led_off()   (SHELL_LEDPORT&=~SHELL_LED)
#else
#define led_on()   (SHELL_LEDPORT&=~SHELL_LED)
#define led_off()  (SHELL_LEDPORT|=SHELL_LED)
#endif
#else
#error "No LED Defined in config.h"
#endif
#else
//#warning "NO LED configured - create LED-Dummy"
#define led_init()
#define led_on()
#define led_off()
#endif

void shell_init(void);
void shell_loop(void);
uint8_t shell_poll(void);  // optional function running in idle time
#ifdef __cplusplus
}
#endif
