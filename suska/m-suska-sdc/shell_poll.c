#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "../suska-iii/joystick.h"
#include "../suska-iii/buttons.h"
#if defined(SUSKA_BF)|defined(SUSKA_B)
#include "../suska-iii/keyboard.h"
#include "../suska-iii/mouse.h"
extern uint8_t kb_available;
extern uint8_t ms_available;
#endif
#include "../uart-irq/uart-irq.h"
#include "../spi/spi.h"
#ifdef USE_SUSKASPI
#include "../suska-iii/suskaspi.h"
#endif
#include "../mmc/mmc.h"
#include "../tff/ff.h"
#include "../suska-iii/sdrawfile.h"

uint8_t shell_poll(void)
{
 uint8_t res; 
#if defined(SUSKA_B) | defined(SUSKA_BF)
    joystick_poll();
    res=buttons_poll();
    if (kb_available)
    {
	    keyboard_poll();
    }
    if (ms_available)
    {
	    mouse_poll();
    }
#if defined(SUSKA_BF)
    command_poll();
#endif
#endif
#if defined(SD_IMAGEFILE)
    sdraw_poll();
#endif
    return res;
}
