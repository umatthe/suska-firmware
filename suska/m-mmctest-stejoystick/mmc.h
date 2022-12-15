#ifndef _MMC_H_
#define _MMC_H_

#include "config.h"
#ifndef NOAVRLIB
#include <avr/io.h>
#endif
#include <stdint.h>


#include "mmc-hw.h"

uint8_t mmc_init(void);

uint8_t mmc_read_sector (uint32_t ,uint8_t *);

uint8_t mmc_write_sector (uint32_t ,const uint8_t *);

uint8_t mmc_write_command (uint8_t cmd, uint32_t arg);

uint8_t mmc_wait_ready(void);

uint8_t mmc_get_CardType(void);

void mmc_dump_buffer(uint8_t * ,uint16_t);

// PFF Interfacces
uint8_t mmc_read_part (uint32_t blocknr, uint8_t *Buffer,uint16_t ofs, uint16_t cnt);
uint8_t mmc_write_part ( const uint8_t *buff, uint32_t blocknr);

/* MMC card types */
#define CT_MMC3         0x01            /* MMC ver 3 */
#define CT_MMC4         0x02            /* MMC ver 4+ */
#define CT_MMC          0x03            /* MMC */
#define CT_SDC1         0x04            /* SD ver 1 */
#define CT_SDC2         0x08            /* SD ver 2+ */
#define CT_SDC          0x0C            /* SD */
#define CT_BLOCK        0x10            /* Block addressing */

#endif //_MMC_H_
