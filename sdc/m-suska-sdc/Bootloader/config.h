#include "../config.h"
// SPI config
#undef SPI_TIMEOUT
#undef SPI_USE_CHUNK

// Bootloader Config
#define USE_FLASH_LED
#define FLASH_LED_PORT PORTB
#define FLASH_LED_DDR DDRB
#define FLASH_LED_PIN PB0
#define FLASH_LED_POLARITY 1
#define ALWAYS_UPDATE_VERSION_00_00
#undef USE_POWER_SWITCH
#undef USE_DISPLAY

