#include <avr/io.h>
#include <string.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>
#include "fat1216.h"
#include "config.h"
#define LEDOFF() FLASH_LED_PORT |= 1<<FLASH_LED_PIN
#define LEDON() FLASH_LED_PORT &= ~(1<<FLASH_LED_PIN)


typedef struct
{
	uint32_t dev_id;
	uint16_t app_version;
	uint16_t crc;
} bootldrinfo_t;


uint16_t startcluster;
uint16_t updatecluster=0; //is set when update is available
bootldrinfo_t current_bootldrinfo;

void (*app_start)(void) = 0x0000;

static inline void check_file(void)
{
	//Check filesize
	
	if (filesize != FLASHEND - BOOTLDRSIZE + 1)
		return;

	bootldrinfo_t *file_bootldrinfo;
	fat1216_readfilesector(startcluster, (FLASHEND - BOOTLDRSIZE + 1) / 512 - 1);
	
	file_bootldrinfo =  (bootldrinfo_t*) (uint8_t*) (fat_buf + (FLASHEND - BOOTLDRSIZE - sizeof(bootldrinfo_t) + 1) % 512);
	
	//Check DEVID
	if (file_bootldrinfo->dev_id != DEVID)
		return;
		
	//Check application version
	#ifdef ALWAYS_UPDATE_VERSION_00_00
	if ((file_bootldrinfo->app_version <= current_bootldrinfo.app_version) && (file_bootldrinfo->app_version!=0x0000))
		return;
	#else
	if (file_bootldrinfo->app_version <= current_bootldrinfo.app_version)
		return;
	#endif

	current_bootldrinfo.app_version = file_bootldrinfo->app_version;
	updatecluster = startcluster;
}

int main(void)
{
	uint16_t i;
	
	uint16_t filesector, j;
	uint16_t *lpword;
	uint16_t adr;
  
        #ifdef USE_POWER_SWITCH
	POWER_SWITCH_DDR |= 1<<POWER_SWITCH_PIN;
	POWER_SWITCH_PORT |= 1<<POWER_SWITCH_PIN;
        #endif

        #ifdef USE_DISPLAY
	_DISPLAY_DDR_  |= (1<<_DISPLAY_CD_) | (1<<_DISPLAY_RESET_) | (1<<_DISPLAY_CS_);
	_DISPLAY_PORT_ &= ~(1<<_DISPLAY_RESET_);
	_DISPLAY_PORT_ |= (1<<_DISPLAY_CD_) | (1<<_DISPLAY_CS_);
        #endif

	//LED On
	#ifdef USE_FLASH_LED
	FLASH_LED_DDR |= 1<<FLASH_LED_PIN;
	#if !FLASH_LED_POLARITY
	FLASH_LED_PORT |= 1<<FLASH_LED_PIN;
	#endif
	#endif
	
	memcpy_P(&current_bootldrinfo, (uint8_t*) FLASHEND - BOOTLDRSIZE - sizeof(bootldrinfo_t) + 1, sizeof(bootldrinfo_t));
	
	if (current_bootldrinfo.app_version == 0xFFFF)
		current_bootldrinfo.app_version = 0; //application not flashed yet
		
	if (fat1216_init() == 0)
	{
		for (i=0; i<512; i++)
		{
			startcluster = fat1216_readRootDirEntry(i);

			if (startcluster == 0xFFFF)
				continue;
			
			check_file();
		}
		
		if (updatecluster)
		{
			for (filesector = 0; filesector < (FLASHEND - BOOTLDRSIZE + 1) / 512; filesector++)
			{
				#ifdef USE_FLASH_LED
				FLASH_LED_PORT ^= 1<<FLASH_LED_PIN;
				#endif
		
				lpword = (uint16_t*) fat_buf;
				fat1216_readfilesector(updatecluster, filesector);
	
				for (i=0; i<(512 / SPM_PAGESIZE); i++)
				{
					adr = (filesector * 512) + i * SPM_PAGESIZE;
					boot_page_erase(adr);
					while (boot_rww_busy())
						boot_rww_enable();
			
					for (j=0; j<SPM_PAGESIZE; j+=2)
						boot_page_fill(adr + j, *lpword++);

					boot_page_write(adr);
					while (boot_rww_busy())
						boot_rww_enable();
				}
			}
	
			//LED on
			#ifdef USE_FLASH_LED
			#if FLASH_LED_POLARITY
			FLASH_LED_PORT &= ~(1<<FLASH_LED_PIN);
			#else
			FLASH_LED_PORT |= 1<<FLASH_LED_PIN;
			#endif
			#endif
		}
	}
	
	unsigned short flash_crc = 0xFFFF;
	
	for (adr=0; adr<FLASHEND - BOOTLDRSIZE + 1; adr++)
		flash_crc = _crc_ccitt_update(flash_crc, pgm_read_byte(adr));
		
	if (flash_crc == 0)
	{
		//Led off
		#ifdef USE_FLASH_LED
		FLASH_LED_PORT &= ~(1<<FLASH_LED_PIN);
		FLASH_LED_DDR = 0x00;
		#endif
		app_start();
	}
	
	while (1);
}
