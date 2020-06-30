//
// isDigit.c
//
// uint8_t isDigit(uint8_t c)
//
// returns 1 if digit, else 0
//

#include "isDigit.h"

__attribute__((noinline)) uint8_t isDigit(uint8_t c)
{
    if ((c >= '0') && (c<= '9'))
        return(1);
    else
        return(0);
}
