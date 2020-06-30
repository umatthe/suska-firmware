
/////////////////////////////////////
//                                 //
// Udo Matthe   24.06.2017         //
//                                 //
/////////////////////////////////////


#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "config.h"

#include "../timer/tick.h"
#include "../misc/itoa.h"
#include "../uart-irq/uart-irq.h"


#ifdef USE_SUSKASPI
#include "../suska-iii/suskaspi.h"
#else
#include "../spi/spi.h"
#endif

#include "hidhandler.h"

extern hid_connectionStatus keyboard;          // create keyboard

extern uint32_t tracelevel;


void usbloop(void)
{

        uint32_t version;

#ifdef USE_SUSKASPI
        Suskaspi_init();
#else
	spi_init();
#endif

        hid_init( &keyboard);                                   // Initialise USB host shield.
        printf("hid_init done\n");

        // Create a new HID keyboard handler,
        hid_setEventHandler( hidEventHandler);

        uint32_t ta=getTicks(50);

        setTickSignal(ta);
        while(1)
        {
                if(tickReached())
                {
                        hid_poll();
                        setTickSignal(ta);
                }
        }


}
