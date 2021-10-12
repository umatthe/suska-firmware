//#define USE_SUSKASPI
//#define SuskaSPI_DDR    DDRD
//#define SuskaSPI_PORT   PORTD
//#define SuskaSPI_PIN    PIND
//#define SuskaSPI_MOSI   PD7
//#define SuskaSPI_MISO   PD6
//#define SuskaSPI_SCK    PD5
//#define SuskaSPI_SS1    PD4
//#define SuskaSPI_SS0    PD3
//#define Suska_INT_DDR   DDRD
//#define Suska_INT_PORT   PORTC
//#define Suska_INT_BIT       PC0



#ifndef __ARDUINO_USB__
#define __ARDUINO_USB__
#include "../../../suska-iii/suskaspi.h"

#ifdef MAX_INT_DDR 
#warning "MAX_INT_DDR defined"
#undef MAX_INT_DDR
#endif
#define MAX_INT_DDR Suska_INT_DDR

#ifdef MAX_INT_BIT
#warning "MAX_INT_BIT defined"
#undef MAX_INT_BIT
#endif
#define MAX_INT_BIT  Suska_INT_BIT

#ifdef MAX_INT_PORT
#warning "MAX_INT_PORT defined"
#undef MAX_INT_PORT
#endif
#define MAX_INT_PORT  Suska_INT_PORT

#ifdef MAX_NORESET
#warning "Arduino Shield should have have RESET enabled"
#endif
#ifndef MAX_NOGPX
#warning "Arduino Shield has no GPX - disabled"
#define MAX_NOGPX
#endif
#endif
