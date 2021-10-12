#ifndef __HID__
#define __HID__
#include "hid.h"
#include "../devices/spi/max3421e/max3421e_constants.h"
#include "../devices/spi/max3421e/max3421e.h"
#include "../devices/spi/max3421e/max3421e_usb.h"

void hidEventHandler(hid_connectionStatus * connection, hid_eventType event, uint16_t length, hid_bootReport * data);
#endif
