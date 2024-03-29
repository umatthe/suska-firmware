#include <avr/io.h>
#include "mmc_lib.h"
#include "fat1216.h"
#include <string.h>

#include <stdlib.h>
#include <util/delay.h>


#define LEDOFF() FLASH_LED_PORT |= 1<<FLASH_LED_PIN
#define LEDON() FLASH_LED_PORT &= ~(1<<FLASH_LED_PIN)


uint8_t fat_buf[512];
uint32_t filesize;

static uint16_t  RootDirRegionStartSec;
static uint32_t  DataRegionStartSec;
static uint16_t  RootDirRegionSize;
static uint8_t   SectorsPerCluster;
static uint16_t  FATRegionStartSec;
static uint8_t   part_type;

static uint16_t currentfatsector;

/*
void blink(void)
{
int i;
for(i=0;i<10;i++)
{
LEDOFF();
 _delay_ms(500.0);
LEDON();
 _delay_ms(500.0);
}
}
*/

unsigned char fat1216_init(void)
{
	mbr_t *mbr = (mbr_t*) fat_buf;
	vbr_t *vbr = (vbr_t*) fat_buf;
	
	uint8_t init = 1, i;
	for (i=0; (i<10) && (init != 0); i++)
		init = mmc_init();
	if (init != MMC_OK)
		return 1;
	//Load MBR
	mmc_start_read_block(0);
	mmc_read_buffer();
	mmc_stop_read_block();
	
	if (mbr->sector.magic != 0xAA55)
		return 2;
		
	//Try sector 0 as MBR
	
	if (((mbr->sector.partition[0].typeId == PARTTYPE_FAT12) || (mbr->sector.partition[0].typeId == PARTTYPE_FAT16L)) && ((mbr->sector.partition[0].state == 0x00) || (mbr->sector.partition[0].state == 0x80)))
	{
		FATRegionStartSec = mbr->sector.partition[0].sectorOffset;
		//Load VBR
		mmc_start_read_block(FATRegionStartSec);
		mmc_read_buffer();
		mmc_stop_read_block();
	}
	else
		FATRegionStartSec = 0;
	
	//Check VBR
	if ((vbr->bsFileSysType[0] != 'F') || (vbr->bsFileSysType[3] != '1'))
		return 2;
		
	if (vbr->bsFileSysType[4] == '2')
		part_type = PARTTYPE_FAT12;
	else
		if (vbr->bsFileSysType[4] == '6')
			part_type = PARTTYPE_FAT16L;
		else
			return 2;
	
	
	SectorsPerCluster  			= vbr->bsSecPerClus;		// 8
	
	// Calculation Algorithms
	FATRegionStartSec			+= vbr->bsRsvdSecCnt;						// 6
	RootDirRegionStartSec	 	= FATRegionStartSec + (vbr->bsNumFATs * vbr->bsNrSeProFAT16);		// 496	
	RootDirRegionSize		 	= vbr->bsRootEntCnt / 16;						// 32
	DataRegionStartSec 			= RootDirRegionStartSec + RootDirRegionSize;	// 528
	
	return 0;
}

uint16_t fat1216_readRootDirEntry(uint16_t entry_num)
{
	direntry_t *dir; //Zeiger auf einen Verzeichniseintrag
	
	if ((entry_num / 16) >= RootDirRegionSize)
		return 0xFFFF; //End of root dir region reached!
	
	//uint32_t dirsector = RootDirRegionStartSec + entry_num * sizeof(direntry_t) / 512;
	uint32_t dirsector = RootDirRegionStartSec + entry_num / 16;
	entry_num %= 512 / sizeof(direntry_t);
	
	mmc_start_read_block(dirsector);
	mmc_read_buffer();
	mmc_stop_read_block();

	dir = (direntry_t *) fat_buf + entry_num;

	if ((dir->name[0] == 0) || (dir->name[0] == 0xE5) || (dir->fstclust == 0))
		return 0xFFFF;
		
	filesize = dir->filesize;
	
	return dir->fstclust;
}

static void load_fat_sector(uint16_t sec)
{
	if (currentfatsector != sec)
	{
		mmc_start_read_block(FATRegionStartSec + sec);
		mmc_read_buffer();
		mmc_stop_read_block();
		currentfatsector = sec;
	}
}


void fat1216_readfilesector(uint16_t startcluster, uint16_t filesector)
{
	uint16_t clusteroffset;
	uint8_t temp, secoffset;
	uint32_t templong;
	
	fatsector_t *fatsector = (fatsector_t*) fat_buf;
	
	clusteroffset = filesector; // seit uint16_t filesector gehts nun doch ;)
	temp = SectorsPerCluster >> 1;
	while(temp)
	{
		clusteroffset >>= 1;
		temp >>= 1;
	}

	secoffset = (uint8_t)filesector & (SectorsPerCluster-1); // SectorsPerCluster is always power of 2 !
	
	currentfatsector = 0xFFFF;
	
	while (clusteroffset)
	{
		if (part_type == PARTTYPE_FAT16L)
		{//FAT16
			load_fat_sector(startcluster / 256);
			startcluster = fatsector->fat16_entries[startcluster % 256];
		}
		
		if (part_type == PARTTYPE_FAT12)
		{//FAT12
			uint32_t fat12_bytenum;
			uint8_t clust1, clust2;
	
			fat12_bytenum = (startcluster * 3 / 2);
			
			load_fat_sector(fat12_bytenum / 512);
			clust1 = fatsector->fat_bytes[fat12_bytenum % 512];
			fat12_bytenum++;
			load_fat_sector(fat12_bytenum / 512);
			clust2 = fatsector->fat_bytes[fat12_bytenum % 512];
		
			if (startcluster & 0x01)
				startcluster = ((clust1 & 0xF0) >> 4) | (clust2 << 4);
			else
				startcluster = clust1 | ((clust2 & 0x0F) << 8);
		}
		
		clusteroffset--;
	}
	
	if (startcluster < 2)
		return; //Free/reserved Sector
	
	if ((startcluster & 0xFFF0) == 0xFFF0) //FAT16 hat nur 16Bit Clusternummern
		return; //Reserved / bad / last cluster

	templong = startcluster - 2;
	temp = SectorsPerCluster >> 1;
	while(temp)
	{
		templong <<= 1;
		temp >>= 1;
	}
		
	mmc_start_read_block(templong + DataRegionStartSec + secoffset);
	mmc_read_buffer();
	mmc_stop_read_block();
}
