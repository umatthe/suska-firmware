#include <avr/io.h>
#include <util/delay.h>
#include "mmc_lib.h"
#include "fat1216.h"
#include "config.h"
#include "../../spi/spi.h"

#define LEDOFF() FLASH_LED_PORT |= 1<<FLASH_LED_PIN
#define LEDON() FLASH_LED_PORT &= ~(1<<FLASH_LED_PIN)

static unsigned char cmd[6];

static uint8_t send_cmd(void)
{
	uint8_t i;
	
	spi_send(0xFF); //Dummy delay 8 clocks
	
	MMC_PORT &= ~(1<<MMC_CS);	//MMC Chip Select -> Low (activate)

	for (i=0; i<6; i++)
	{//Send 6 Bytes
		spi_send(cmd[i]);
	}
	
	uint8_t result;
	
	for(i=0; i<128; i++)
	{//waiting for response (!0xff)
		result=spi_send(0xFF);
		
		if ((result & 0x80) == 0)
			break;
	}
	
	return(result); // TimeOut !
}

uint8_t mmc_init(void)
{
	MMC_DDR |= 1<<MMC_CS;	//MMC Chip Select -> Output

        spi_init();
	spi_enable_slow();

	uint8_t i;
	
	i = 12;
	while (i)
	{//Pulse 80+ clocks to reset MMC
		spi_send(0xFF);
		i--;
	}

	
	uint8_t res;

	cmd[0] = 0x40 + MMC_GO_IDLE_STATE;
	cmd[1] = 0x00; cmd[2] = 0x00; cmd[3] = 0x00; cmd[4] = 0x00; cmd[5] = 0x95;
	cmd[5] = 0x95;
	
	for (i=0; i<MMC_CMD0_RETRIES; i++)
	{
		res = send_cmd(); //store result of reset command, should be 0x01
		
		MMC_PORT |= 1<<MMC_CS; //MMC Chip Select -> High (deactivate);

		spi_send(0xFF);
		if (res == 0x01)
			break;
	}
	
	if (i == MMC_CMD0_RETRIES)
		return MMC_CMD0_TIMEOUT;
	if (res != 0x01) //Response R1 from MMC (0x01: IDLE, The card is in idle state and running the initializing process.)
		return(MMC_INIT);
	
	cmd[0]=0x40 + MMC_SEND_OP_COND;
	
	i=0;
	
	while((send_cmd() != 0) && (i < 0xFF))
	{
		MMC_PORT |= 1<<MMC_CS; //MMC Chip Select -> High (deactivate);
		_delay_ms(1.0);
		spi_send(0xFF);
		i++;
	}
	
	MMC_PORT |= 1<<MMC_CS; //MMC Chip Select -> High (deactivate);
	
	if (i < 0xFF)
		return(MMC_OK);
	return(MMC_OP_COND_TIMEOUT);
}

static uint8_t wait_start_byte(void)
{
	uint8_t i = 255;
	
	do
	{
		if ( spi_send(0xFF) == 0xFE)
			return MMC_OK;
		
		i--;
	} while (i);
	
	return MMC_NOSTARTBYTE;
}

uint8_t mmc_start_read_block(uint32_t adr)
{
	adr <<= 1;
	
	cmd[0] = 0x40 + MMC_READ_SINGLE_BLOCK;
	cmd[1] = (adr & 0x00FF0000) >> 0x10;
	cmd[2] = (adr & 0x0000FF00) >> 0x08;
	cmd[3] = (adr & 0x000000FF);
	cmd[4] = 0;
	
        spi_enable();

	if (send_cmd() != 0x00)
	{
		MMC_PORT |= 1<<MMC_CS; //MMC Chip Select -> High (deactivate);
		return(MMC_CMDERROR); //wrong response!
	}
	
	if (wait_start_byte())
	{
		MMC_PORT |= 1<<MMC_CS; //MMC Chip Select -> High (deactivate);
		return MMC_NOSTARTBYTE;
	}
	
	return(MMC_OK);
}

void mmc_read_buffer(void)
{
	unsigned char *buf = fat_buf;
	unsigned short len = 512;
	while (len--)
	{
		*(buf++) = spi_send(0xFF);
	}
}

void mmc_stop_read_block(void)
{
	//read 2 bytes CRC (not used);
	spi_send(0xFF);
	spi_send(0xFF);
	MMC_PORT |= 1<<MMC_CS; //MMC Chip Select -> High (deactivate);
}
