// SDC-Controller Suska-III-C/BF/B
#include "avr-names.h"

#define SWVERSION 0x20240824

#if defined( AVRCORE )
#include "config.h.avrcore"
#elif defined( ARDUINO )
#include "config.h.arduino"
#else
#error "unknown Board"
#endif
#define ATARIKEYBUBRR (F_CPU/(7812 * 16L)-1)
