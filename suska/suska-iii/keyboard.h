
#include <stdint.h>
#include "config.h"
#ifdef SUART_TXD
#include "../softuart/softuart.h"
#else
#include "../uart-irq/uart-irq.h"
#endif

#define KB_STATUS_INDICATORS    0xED
#define KB_READ_ID	        0xF2
#define KB_TYPEMATIC_RATE_DELAY	0xF3
#define KB_ENABLE               0xF4
#define KB_RESET                0xFF

void keyboard_init(void);
void keyboard_clear_buffer(void);
uint8_t keyboard_send_cmd(uint8_t cmd);
uint8_t keyboard_get(void);
void keyboard_poll(void);
uint8_t keyboard_send_byte(uint8_t data);
void keyboard_put(uint8_t c);
uint8_t is_keyboard_available(void);
void command_poll(void);
void keyboard_sendspi(uint8_t sendkey, uint8_t key);
