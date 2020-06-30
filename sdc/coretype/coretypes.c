#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "coretypes.h"

uint8_t coretype    = 0xAF;
uint8_t coresubtype = 0xFE;
