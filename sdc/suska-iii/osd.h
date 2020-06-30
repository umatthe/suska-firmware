//
// OnScreenDisplay Driver
// Derived from Mist/Minimig Project
//
// 20200321 Udo Matthe: Initial Version
//
#ifndef OSD_H_INCLUDED
#define OSD_H_INCLUDED

/*constants*/
#define OSDCTRLUP        0x01        /*OSD up control*/
#define OSDCTRLDOWN      0x02        /*OSD down control*/
#define OSDCTRLSELECT    0x04        /*OSD select control*/
#define OSDCTRLMENU      0x08        /*OSD menu control*/
#define OSDCTRLRIGHT     0x10        /*OSD right control*/
#define OSDCTRLLEFT      0x20        /*OSD left control*/

// some constants
#define OSDNLINE         8           // number of lines of OSD
#define OSDLINELEN       256         // single line length in bytes

// ---- old Minimig v1 constants -------
#define MM1_OSDCMDREAD     0x00      // OSD read controller/key status
#define MM1_OSDCMDWRITE    0x20      // OSD write video data command
#define MM1_OSDCMDENABLE   0x41      // OSD enable command
#define MM1_OSDCMDDISABLE  0x40      // OSD disable command
#define MM1_OSDCMDRST      0x80      // OSD reset command
#define MM1_OSDCMDAUTOFIRE 0x84      // OSD autofire command
#define MM1_OSDCMDCFGSCL   0xA0      // OSD settings: scanlines effect
#define MM1_OSDCMDCFGIDE   0xB0      // OSD enable HDD command
#define MM1_OSDCMDCFGFLP   0xC0      // OSD settings: floppy config
#define MM1_OSDCMDCFGCHP   0xD0      // OSD settings: chipset config
#define MM1_OSDCMDCFGFLT   0xE0      // OSD settings: filter
#define MM1_OSDCMDCFGMEM   0xF0      // OSD settings: memory config
#define MM1_OSDCMDCFGCPU   0xFC      // OSD settings: CPU config

#define OSD_ARROW_LEFT 1
#define OSD_ARROW_RIGHT 2


/*functions*/
void OsdSetTitle(char *s);
void OsdWrite(unsigned char n, char *s, unsigned char inver);
void OsdWriteOffset(unsigned char n, char *s, unsigned char inver, char offset);
#endif
