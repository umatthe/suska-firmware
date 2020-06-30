#include <avr/io.h>

typedef struct
{
	unsigned long dev_id;
	unsigned short app_version;
	unsigned short crc;
} bootldrinfo_t;

const bootldrinfo_t bootlodrinfo __attribute__ ((section (".bootldrinfo"))) = {DEVID, SWVERSIONMAJOR << 8 | SWVERSIONMINOR, 0x0000};



