// SDC-Controller Suska-III-C/BF/B
#include "avr-names.h"

#define SWVERSION 0x20211009

#define _USE_IRQ_

#if defined( SUSKA_BF )
#include "config.h.suska-bf"
#elif defined( SUSKA_C )
#include "config.h.suska-c"
#elif defined( SUSKA_B )
#include "config.h.suska-b"
#else
#error "unknown Suska Board"
#endif
#define ATARIKEYBUBRR (F_CPU/(7812 * 16L)-1)
