/*
 i2a.h
 This is a simple implementation of
 converting float into ascii readable output
 Currently restricted to 1 digit after decimal point and
 4 leading digits, like 9999.9
 The space for the ascii string is allocated locally (8 bytes)

 It avoides flash consuming inlcusion of libmath and others

    Stefan Weigl Aug 2007

*/
#ifndef _I2A_H_
#define _I2A_H_ 1

#ifndef noinline
#define noinline __attribute__((noinline))
#endif

#define F2S1MAXLEN 8

const char* f2a(float fl);

#endif // _I2A_H_
