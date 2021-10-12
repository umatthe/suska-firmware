
#include "config.h"
#include "../suska-iii/joystick.h"
#include "../suska-iii/buttons.h"
#if defined(SUSKA_BF)|defined(SUSKA_B)
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
#if defined(SUSKA_C)
#warn "Suska-c no polling"
#endif
    return res;
}

