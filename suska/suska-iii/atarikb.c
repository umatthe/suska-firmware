
/////////////////////////////////////
//                                 //
// Kommandozeilen-Parser           //
//                                 //
// Udo Matthe   05.01.2015         //
//                                 //
/////////////////////////////////////

#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "config.h"

#include "../uart-irq/uart-irq.h"
#include "../suska-iii/suskaspi.h"
#include "../coretype/coretypes.h"
#include "../timer/tick.h"

//#ifdef avrcore
void shell_kb(void)
{
    static char c, send_key_code=0;

    uart_puts_P("Atari-Keyboard\n\r");
    
    c=0x39;

    if(coretype==CT_ARCADE) 
    {
        keyboard_sendspi(send_key_code,c);
        delayms(100);
        keyboard_sendspi(send_key_code,0x80|c);

    }
}
//#endif

