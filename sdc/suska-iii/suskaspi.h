#ifndef __SUSKASPI__
#define __SUSKASPI__

// HW - Modification momentan nicht benutzt
// Draht  PD2 <-> GPO (X18) FPGA nCSO == W20

#include <avr/io.h>
#include <stdint.h>
#ifdef SUSKA_C
#define SS_DISABLE            SuskaSSSPI_PORT|=_BV(SuskaSPI_SS2)|_BV(SuskaSPI_SS1)|_BV(SuskaSPI_SS0)    // 111
#define SS_ENABLEMAX          SuskaSSSPI_PORT&=~_BV(SuskaSPI_SS1)                                       // 101
#define SS_ENABLEVER          SuskaSSSPI_PORT&=~_BV(SuskaSPI_SS2)                                       // 011
#define SS_ENABLESD           SuskaSSSPI_PORT&=~(_BV(SuskaSPI_SS2)|_BV(SuskaSPI_SS1)|_BV(SuskaSPI_SS0)) // 000
#define SS_ENABLEFLASHBOOT    SuskaSSSPI_PORT|=_BV(SuskaSPI_SS2)|_BV(SuskaSPI_SS1)|_BV(SuskaSPI_SS0)    // 111
#else //BF-Board
#define SS_DISABLE            SuskaSSSPI_PORT|=_BV(SuskaSPI_SS2)|_BV(SuskaSPI_SS1)|_BV(SuskaSPI_SS0)    // 111
#define SS_ENABLEJOY          SuskaSSSPI_PORT&=~_BV(SuskaSPI_SS1)                                       // 101
#define SS_ENABLEKEY          SuskaSSSPI_PORT&=~(_BV(SuskaSPI_SS1)|_BV(SuskaSPI_SS0))                   // 100
#define SS_ENABLEVER          SuskaSSSPI_PORT&=~(_BV(SuskaSPI_SS2))                                     // 011
#define SS_ENABLESD           SuskaSSSPI_PORT&=~(_BV(SuskaSPI_SS0)|_BV(SuskaSPI_SS1)|_BV(SuskaSPI_SS2)) // 000
#define SS_ENABLEFLASHBOOT    SuskaSSSPI_PORT|=_BV(SuskaSPI_SS2)|_BV(SuskaSPI_SS1)|_BV(SuskaSPI_SS0)    // 111
#endif

#ifdef BOOT_SD_AVR_EN
#define BOOTAVR_DISABLE      BOOT_AVREN_PORT|=_BV(BOOT_SD_AVR_EN) 
#define BOOTAVR_ENABLE       BOOT_AVREN_PORT&=~_BV(BOOT_SD_AVR_EN) 
#else
#define BOOTAVR_DISABLE
#define BOOTAVR_ENABLE
#endif

void Suskaspi_init(void);
uint8_t Suskaspi_send(uint8_t data);
uint16_t Suskaspi_send16(uint16_t data);
uint32_t readFpgaVersion(uint16_t *type);
uint8_t waitsdreq(void);
uint8_t waitbreq(void);
void sendack(void);
uint16_t sendfb(uint16_t v);
uint32_t Suskaspi_send24(uint32_t data);
void Suskaspi_send_n(unsigned char value, unsigned short cnt);

#define spi_osd_cmd(c) SS_ENABLESD; Suskaspi_send(c); SS_DISABLE
#define spi_osd_cmd_cont(c) SS_ENABLESD; Suskaspi_send(c)
#define OSD_SHOW() SS_ENABLESD; Suskaspi_send(0x41); SS_DISABLE
#define OSD_HIDE() SS_ENABLESD; Suskaspi_send(0x40); SS_DISABLE
#define DisableOsd()  SS_DISABLE
#define spi16(v) Suskaspi_send16(v)
#define spi8(v) Suskaspi_send(v)
#define spi_n(v,n) Suskaspi_send_n(v,n)
#endif
