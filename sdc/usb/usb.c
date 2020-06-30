/*
Copyright 2011 Niels Brouwers

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "../devices/spi/max3421e/max3421e_constants.h"
#include "../devices/spi/max3421e/max3421e_usb.h"

#include "usb.h"

static usb_eventHandler * eventHandler;

void usb_setEventHandler(usb_eventHandler * handler)
{
	eventHandler = handler;
}

/**
 * Fires a USB event. This calls the callback function set by usb_setEventHandler.
 *
 * @param device the device the events relates to.
 * @param event event type (i.e. connected, disconnected)
 */
void usb_fireEvent(usb_device * device, usb_eventType event)
{
	eventHandler(device, event);
}

void usb_initEndPoint(usb_endpoint * endpoint, uint8_t address)
{
	endpoint->address = address;
	endpoint->sendToggle = bmSNDTOG0;
	endpoint->receiveToggle = bmRCVTOG0;
}


/**
 * Print USB device information.
 */
int8_t usb_printDeviceInfo(usb_device * device)
{
	int8_t rcode;
    uint8_t buf[MAX_BUF_SIZE];
    usb_deviceDescriptor deviceDescriptor;

    printf("\n\nDevice address: %d\n", device->address);
    printf("Device active: %d\n", device->active);
    printf("Device firstStringLanguage: %d\n\n", device->firstStringLanguage);

    // Read the device descriptor
    rcode = usb_getDeviceDescriptor(device, &deviceDescriptor);
    if (rcode)
    {
    	printf("getDeviceDescriptor read error: %x\n\n", rcode);
    	return rcode;
    }

    printf("Descriptor Length: %d\n", deviceDescriptor.bLength);
    printf("Descriptor Type: %d\n", deviceDescriptor.bDescriptorType);
    printf("USB BCD: %d\n", deviceDescriptor.bcdUSB);

    printf("Device class: %d\n", deviceDescriptor.bDeviceClass);
    printf("Device subclass: %d\n", deviceDescriptor.bDeviceSubClass);
    printf("Device protocol: %d\n", deviceDescriptor.bDeviceProtocol);
    printf("Maximum Packet: %d\n", deviceDescriptor.bMaxPacketSize0);
    
    printf("Vendor ID: 0x%x\n", deviceDescriptor.idVendor);
    printf("Product ID: 0x%x\n", deviceDescriptor.idProduct);
    printf("Configuration count: %d\n\n", deviceDescriptor.bNumConfigurations);
    
    if (usb_getString(device, deviceDescriptor.iManufacturer, device->firstStringLanguage, MAX_BUF_SIZE, buf) > 0)
    {
    	printf("Manufacturer: %d : %s\n", deviceDescriptor.iManufacturer, buf);
    }
    if (usb_getString(device, deviceDescriptor.iProduct, device->firstStringLanguage, MAX_BUF_SIZE, buf) > 0)
    {
    	printf("Product: %d : %s\n", deviceDescriptor.iProduct, buf);
    }
    if (usb_getString(device, deviceDescriptor.iSerialNumber, device->firstStringLanguage, MAX_BUF_SIZE, buf) > 0)
    {
    	printf("Serial number: %d : %s\n\n", deviceDescriptor.iSerialNumber, buf);
    }

    return 0;
}

int8_t usb_initDevice(usb_device * device, uint8_t configuration)
{
	uint8_t buf[4];

	uint8_t rcode;

	// Set the configuration for this USB device.
	rcode = usb_setConfiguration(device, configuration);
	if (rcode < 0) return rcode;

	// Get the first supported language.
	rcode = usb_getString(device, 0, 0, 4, buf);
	if (rcode < 0) return rcode;
    device->firstStringLanguage = (buf[3] << 8) | buf[2];

    return rcode;
}
