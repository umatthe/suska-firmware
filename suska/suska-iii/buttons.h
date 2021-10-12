#include "config.h"
#include <stdint.h>

#ifdef SUSKA_C
// SUSKA-C SDC-CRTL cannot read buttons
// Dummy always returns 0
#define buttons_poll() 0
#else
void buttons_init(void);
uint8_t buttons_poll(void);
#endif
