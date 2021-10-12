// SDC-Controller Suska-III-C/BF/B
#include "avr-names.h"

#define SWVERSION 0x20201217

#if defined( AVRCORE )
#include "config.h.avrcore"
#elif defined( ARDUINO )
#include "config.h.arduino"
#else
#error "unknown Board"
#endif
