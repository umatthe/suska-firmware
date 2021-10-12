#include "../shared/sbit.h"

#define KB_CLOCK	      PD2  /* Connected to INT0, don't change */
#define KB_CLOCK_PORT		PORTD
#define KB_CLOCK_PIN		PIND
#define KB_CLOCK_DDR		DDRD

#define MS_CLOCK        PD3 /* Connected to INT1, don't change */
#define MS_CLOCK_PORT   PORTD
#define MS_CLOCK_PIN    PIND
#define MS_CLOCK_DDR    DDRD

#define KB_DATA				  PB0 /* Suska Board */
#define KB_DATA_PORT		PORTB
#define KB_DATA_PIN			PINB
#define KB_DATA_DDR			DDRB


#ifdef SUSKA_III_C
#define MS_DATA         PB2 /* Suska Board III-C */
#elif defined SUSKA_III_B
#define MS_DATA         PB1 /* Suska Board */
#endif

#define MS_DATA_PORT    PORTB
#define MS_DATA_PIN     PINB
#define MS_DATA_DDR     DDRB

#ifdef SUSKA_III_C
#define LED_PS2_KEYBOARD_AVAIL	PC4
#define LED_PS2_MOUSE_AVAIL		  PC5
#define LED_PORT			          PORTC
#define LED_DDR				          DDRC

#define JOY_PORT                PORTA
#define JOY_PIN                 PINA
#define JOY_DDR                 DDRA
#endif

#ifdef SUSKA_III_B
#define DEBUG_LED_DDR           SBIT(DDRA,  7)
#define DEBUG_LED               SBIT(PORTA, 7)

#define JOY_PORT                PORTC
#define JOY_PIN                 PINC
#define JOY_DDR                 DDRC

#define JOY_ENA                 SBIT(PORTB, 3)  /* - OK - Port C pin 1, active high */
#define JOY_ENA_DDR             SBIT(DDRB,  3)

#define JOY_BUTTONS_DDR         SBIT(DDRB,  2)
#define JOY_BUTTONS             SBIT(PINB,  2)
#endif

#ifdef SUSKA_III_B
#define OUT_RESET           SBIT(PORTC, 1)  /* - OK - Port C pin 1, active high */
#define OUT_RESET_DDR       SBIT(DDRC,  1)
#define OUT_RESET_CORE      SBIT(PORTC, 0)  /* - OK - Port C pin 0, active high */
#define OUT_RESET_CORE_DDR  SBIT(DDRC,  0)

#define OUT_PS_EN           SBIT(PORTD, 6)  /* - OK - Port D pin 6, active high */
#define OUT_PS_EN_DDR       SBIT(DDRD,  6)

#define OUT_BOOT_ACK        SBIT(PORTB, 4)  /* - OK - Port B pin 4, active high */
#define OUT_BOOT_ACK_DDR    SBIT(DDRB,  4)
#endif
#ifdef SUSKA_III_C
#define OUT_VBIAS_UD        SBIT(PORTD, 4)  /* - OK - Port D pin 4, active high */
#define OUT_VBIAS_UD_DDR    SBIT(DDRD,  4)
#define OUT_VBIAS_CS        SBIT(PORTD, 5)  /* - OK - Port D pin 5, active low */
#define OUT_VBIAS_CS_DDR    SBIT(DDRD,  5)
/* Input port pins */
#define IN_VBIAS_UP         SBIT(PIND,  6)  /* - OK - Port D pin 6, active low */
#define IN_VBIAS_DOWN       SBIT(PIND,  7)  /* - OK - Port D pin 7, active low */

#endif



/* Macro to test data and set port/pin to high if &0x01 else to low */
#define SET_BIT(data, port, pin) ((data & 0x01) ? (port | _BV(pin)) : (port & ~_BV(pin)))
