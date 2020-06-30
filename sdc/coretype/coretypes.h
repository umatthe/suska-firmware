#include "config.h"

extern uint8_t coretype;
extern uint8_t coresubtype;


#ifndef __CORETYPES__
#define __CORETYPES__
// Main Coretype upper 8 Bit 
#define CT_OLD      0xAA
//Subcoretype CT_OLD lower 8 Bit
#define ST_OLD 0xAA

#define CT_SUSKA_C_STE     0x00
//Subcoretype CT_SUSKA_C_STE lower 8 Bit
#define CSTE_000  0x00
#define CSTE_010  0x01
#define CSTE_030L 0x02

#define CT_SUSKA_B_STE     0x01
//Subcoretype CT_SUSKA_B_STE lower 8 Bit
#define BSTE_000  0x00
#define BSTE_010  0x01
#define BSTE_030L 0x02

#define CT_SUSKA_BF_STE    0x02
//Subcoretype CT_SUSKA_BF_STE lower 8 Bit
#define BFSTE_000  0x00
#define BFSTE_010  0x01
#define BFSTE_030  0x02

#define CT_SUSKA_BF_TT     0x03
//Subcoretype CT_SUSKA_BF_TT lower 8 Bit
#define BFTT_030  0x02

#define CT_SUSKA_BF_FALCON 0x04
//Subcoretype CT_SUSKA_BF_FALCON lower 8 Bit
#define FALCON_030  0x02

#define CT_ARCADE   0x10
//Subcoretype CT_ARCADE lower 8 Bit
#define ST_INVADERS 0x00

#define CT_MULTICOMP 0x11
//Subcoretype CT_MULTICOMP lower 8 Bit
#define ST_CPM 0x00

#endif
