// SDC-Controller Suska-III-C/BF/B
#include "avr-names.h"

#define SWVERSION 0x20200620

#if defined( SUSKA_BF )
#include "config.h.suska-bf"
#elif defined( SUSKA_C )
#include "config.h.suska-c"
#elif defined( SUSKA_B )
#include "config.h.suska-c"
#else
#error "unknown Suska Board"
#endif
