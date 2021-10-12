#include "config.h"

extern uint8_t coretype;
extern uint8_t coresubtype;

void show_suska_subtype( void );

#ifndef __CORETYPES__
#define __CORETYPES__
// Main Coretype upper 8 Bit 
#define CT_OLD      0xAA
//Subcoretype CT_OLD lower 8 Bit
#define ST_OLD 0xAA

#define CT_SUSKA_C_STE     0x00
//Subcoretype CT_SUSKA_C_STE lower 8 Bit
#define WF_000  0x00
#define WF_010  0x10
#define WF_030L 0x20
#define WF_030  0x30

#define CT_SUSKA_C_FALCON  0x05
//Subcoretype CT_SUSKA_C_FALCON lower 8 Bit
// See CT_SUSKA_C_STE

#define CT_SUSKA_B_STE     0x01
//Subcoretype CT_SUSKA_B_STE lower 8 Bit
// See CT_SUSKA_C_STE

#define CT_SUSKA_BF_STE    0x02
//Subcoretype CT_SUSKA_BF_STE lower 8 Bit
// See CT_SUSKA_C_STE

#define CT_SUSKA_BF_TT     0x03
//Subcoretype CT_SUSKA_BF_TT lower 8 Bit
// See CT_SUSKA_C_STE

#define CT_SUSKA_BF_FALCON 0x04
//Subcoretype CT_SUSKA_BF_FALCON lower 8 Bit
// See CT_SUSKA_C_STE

#define CT_ARCADE   0x10
//Subcoretype CT_ARCADE lower 8 Bit
#define ST_INVADERS 0x00

#define CT_MULTICOMP 0x11
//Subcoretype CT_MULTICOMP lower 8 Bit
#define ST_CPM 0x00

#define CT_TEST 0x20
//Subcoretype CT_TEST lower 8 Bit
#define ST_AVR 0x00

#endif
