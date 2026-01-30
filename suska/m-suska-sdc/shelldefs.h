/////////////////////////////////////
//                                 //
// Kommandozeilen-Parser           //
//                                 //
// Udo Matthe   12.12.2012         //
//                                 //
/////////////////////////////////////
//
// Konfiguration der YH-Shell 
// Beispiel fuer Suska - SDC
//
/////////////////////////////////////


#ifndef __SHELLDEFS_H__
#define __SHELLDEFS_H__

#include "config.h"

//
// Hier spezielle Includes
// 
#include "../asisp/asisp.h"
#include "../asisp/asappl.h"
#include "../asisp/asshell.h"
#include "../suska-iii/suskashell.h"
#include "../suska-iii/power.h"
#ifdef PUZZLE
void shell_puzzle(uint8_t *rounds);
#endif
#ifdef HAVE_EE_CONFIG
#include "../config/configshell.h"
#endif


//#define F(X) (const __flash char[]) { X }


//
// Anzahl der implementierten Kommandos +2
//
#ifdef HAVE_EE_AS_ENABLE
#define MAXCOMMAND 26+1+2 
#else
#define MAXCOMMAND 26+2+2 /* HAVE_EE_CONFIG ist fix mit 2 dabei */ 
#endif
#define MAXCMDLEN 8 
#define MAXHELPLEN 30

//
// Hinweise zur Kommando-Funktion:
//
// Maximal 3 Parameter erlaubt.
// Die Signatur der (zu implementierenden Funktion):
// - fuer 0 Parameter "void func( void )" 
// - fuer 1 Parameter "void func( uint8_t* )" 
// - fuer 2 Parameter "void func( uint8_t*, uint8_t* )" 
// - fuer 3 Parameter "void func( uint8_t*, uint8_t*, uint8_t* )" 
//

const __flash struct befehle befehle[MAXCOMMAND] =
{
//
// Ab hier Standard-Befehle (aus shell)
//
//       Parameterzahl, Kommando,   Funktion,      Hilfstext
	{0,             F("help"),    shell_help,     F("this Text")},
#ifdef SUSKA_BF
	{0,             F("reset"),    shell_reset,    F("power off")},
#else
  #ifdef __HAVE_FILESYSTEM__
	{0,             F("reset"),    shell_reset,    F("mount SDCard")},
  #else
	{0,             F("reset"),    shell_reset,    F("reset avr")},
  #endif
#endif
#ifndef SUSKA_C_SYSCTRL
	{0,             F("info"),     shell_info,     F("System Info")},
#endif
	{1,             F("trace"),    shell_trace,    F("enable Debug Traces")},
	{1,             F("delay"),    shell_delay,    F("wait deltime ms")},
#ifdef PUZZLE
	{1,             F("puzzle"),   shell_puzzle,   F("puzzle rounds")},
#endif
#ifdef __HAVE_FILESYSTEM__
	{0,             F("ls"),       shell_ls,       F("list SDCard Directory")},
	{1,             F("cat"),      shell_cat,      F("show file")},
	{1,             F("hex"),      shell_hex,      F("hexdump file")},
	{1,             F("do"),       shell_do,       F("exec batchfile")},
#endif 
//
// Reset Befehle
//
#ifdef RESET
	{1,             F("res"),  shell_res,     F("res resetlevel")},
	{1,             F("cres"),  shell_cres,     F("cres coreresetlevel")},
#endif
//
// Ab hier spezielle Befehle (aus asisp)
//
#if defined AS_CONFIG
	{2,             F("as-read"),  shell_asread,   F("read ConfigFlash len/all")},
	{1,             F("as-write"), shell_aswrite,  F("write .rbf file to ConfigFlash")},
	{0,             F("as-getid"), shell_asgetid,  F("ConfigFlash Info")},
	{0,             F("as-off"), shell_aspowerdown,  F("ConfigFlash powerdown")},
#ifdef HAVE_EE_AS_ENABLE
        {1,             F("as-en"), shell_asenable, F("1/0 en/disable as")},
#endif
#endif
//
// Ab hier USB Stuff
//
#if defined USB_GATE | defined USBLOOP
	{0,             F("usb"),      shell_usb,      F("usb Test (endlos)")},
#endif
#ifdef SUSKAFLASH
//
// Ab hier Flash Stuff
//
	{1,             F("f-erase"), shell_ferase,    F("f-erase 512k-Slot/all")},
	{3,             F("f-read"),  shell_fread,     F("f-read offset len tofile")},
	{2,             F("f-write"), shell_fwrite,    F("f-write offset fromfile")},
	{2,             F("f-dump"),  shell_fdump,     F("f-dump offset len")},
#endif

#ifdef HAVE_EE_CONFIG
        {1,             F("c-set"), shell_setconfig, F("c-set Value")},
        {0,             F("c-get"), shell_getconfig, F("c-get")},
#endif
//
// Image File z.B. Multcomp
//
#ifdef SD_IMAGEFILE
	{2,             F("sd"),      shell_sd,        F("sd rawimagefile type")},
#endif
#if defined SUSKA_B | defined SUSKA_BF
	{0,             F("ps2"),  shell_ps2,     F("reset ps2")},
#endif
#ifdef SUSKA_BF
	{1,             F("joy"),  shell_joy,     F("joy value (test only)")},
#endif
#ifdef OSD
	{0,             F("osd"),  shell_osd,     F("shows info on osd")},
#endif
#ifdef SPIRAM
      {2,             F("r-write"), shell_rwrite,    F("r-write offset fromfile")},
#endif
//
// Ende Kennung (nicht loeschen)
//
	{0, 0, NULL,  0 },
};
#endif
