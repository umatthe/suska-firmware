
#include "config.h"
#include "../suska-iii/joystick.h"
#include "../suska-iii/buttons.h"
#if defined(SUSKA_BF)
#include "../suska-iii/keyboard.h"
#include "../suska-iii/mouse.h"
extern uint8_t kb_available;
extern uint8_t ms_available;
#endif

uint8_t shell_poll(void)
{
 uint8_t res; 
#if defined(SUSKA_B) | defined(SUSKA_BF)
    joystick_poll();
#endif
#if defined(SUSKA_BF)
    res=buttons_poll();
    if (kb_available)
    {
	    keyboard_poll();
    }
    if (ms_available)
    {
	    mouse_poll();
    }
    command_poll();
#endif
#if defined(SUSKA_C)
#warn "Suska-c no polling"
#endif
    return res;
}

