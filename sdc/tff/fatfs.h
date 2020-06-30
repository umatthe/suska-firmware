#if !defined(_PFF_) & !defined(_TFF_) & !defined(_FF_)
#define _TFF_
#warning "no definition of _PFF_ or _TFF_ or _FF_ using defaulft _TFF_"
#endif

#if defined(_PFF_)
#include "pff.h"
#elif defined(_TFF_)
#include "tff.h"
#elif defined(_FF_)
#include "ff.h"
#else
#error "_PFF_ or _TFF_ of _FF_ define needed."
#endif
