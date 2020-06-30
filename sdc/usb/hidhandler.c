
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

#include "hidhandler.h"

hid_connectionStatus keyboard;          // create keyboard

extern uint32_t tracelevel;

// Event handler to process incoming data from HID.
void hidEventHandler(hid_connectionStatus * connection, hid_eventType event, uint16_t length, hid_bootReport * data)
{
        printf("Event\n");

        switch (event)
        {
        case HID_DISCONNECT:
                printf(PSTR("HID EVENT DISCONNECT\n"));

                switch (*connection)
                {
                case HID_UNUSED:
                case HID_CLOSED:
                case HID_OPEN:
                case HID_RECEIVING:
                case HID_WRITING:
                case HID_ERROR:
                default:
                        break;
                }

                break;

        case HID_CONNECTION_OPEN: // open verb
                printf(PSTR("HID EVENT CONNECTION OPEN\n"));

                switch (*connection)
                {
                case HID_UNUSED:
                case HID_CLOSED:
                case HID_OPEN:
                case HID_RECEIVING:
                case HID_WRITING:
                case HID_ERROR:
                default:
                        break;
                }

                break;

        case HID_CONNECTION_CLOSE: // close verb
                printf(PSTR("HID EVENT CONNECTION CLOSE\n"));

                switch (*connection)
                {
                case HID_UNUSED:
                case HID_CLOSED:
                case HID_OPEN:
                case HID_RECEIVING:
                case HID_WRITING:
                case HID_ERROR:
                default:
                        break;
                }

                break;

        case HID_CONNECTION_FAIL:
                printf(PSTR("HID EVENT CONNECTION FAILED\n"));

                switch (*connection)
                {
                case HID_UNUSED:
                case HID_CLOSED:
                case HID_OPEN:
                case HID_RECEIVING:
                case HID_WRITING:
                case HID_ERROR:
                default:
                        break;
                }

                break;

        case HID_CONNECTION_RECEIVE:
#ifdef DEBUG
              printf(PSTR("HID EVENT CONNECTION RECEIVE DATA\n"));
#endif
                hid_HIDCodeASCII( data ); // convert the HID codes into ASCII codes. Probably will be doing a lot of this.

                for (uint8_t i = 0; i < 6; ++i)
                {

                        if (*(uint8_t*)&data->keyboard.keycode[i] != '\0')
                        {
                                putchar( *(uint8_t*)&data->keyboard.keycode[i]);
                        }
                        else
                                break;

                }

                switch (*connection)
                {
                case HID_UNUSED:
                case HID_CLOSED:
                case HID_OPEN:
                case HID_RECEIVING:
                case HID_WRITING:
                case HID_ERROR:
                default:
                        break;
                }

                break;
        default:
                {
                        printf("Default\n");
                        break;
                }
        }

}

