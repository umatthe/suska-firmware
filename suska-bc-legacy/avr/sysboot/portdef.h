#include "../shared/sbit.h"

#ifdef SUSKA_III_C

/* Input port pins */
#define IN_RESET_POWER    SBIT(PINB,  2)  /* Port B pin 2, active high */
#define IN_RESET_CORE     SBIT(PINA,  5)  /* Port A pin 5, active high */
#define IN_BOOT_REQ       SBIT(PIND,  3)  /* Port D pin 3, active high */
#define SPI_MISO          SBIT(PINB,  6)  /* Port B pin 6, Master in */
#define FPGA_CONFIG_DONE  SBIT(PINB,  0)  /* Port B pin 0, FPGA configuration done, active high */

/* Output port pins */
#define OUT_RESET           SBIT(PORTC, 3)  /* Port C pin 3, active high */
#define OUT_RESET_DDR       SBIT(DDRC,  3)
#define OUT_RESET_CORE      SBIT(PORTC, 2)  /* Port C pin 2, active high */
#define OUT_RESET_CORE_DDR  SBIT(DDRC,  2)

#define OUT_PS_50         SBIT(PORTD, 7)  /* Port D pin 7, active high */
#define OUT_PS_50_DDR     SBIT(DDRD,  7)
#define OUT_PS_33         SBIT(PORTD, 6)  /* Port D pin 6, active high */
#define OUT_PS_33_DDR     SBIT(DDRD,  6)
#define OUT_PS_12         SBIT(PORTD, 5)  /* Port D pin 5, active high */
#define OUT_PS_12_DDR     SBIT(DDRD,  5)
#define OUT_PS_VBIAS      SBIT(PORTD, 4)  /* Port D pin 4, active high */
#define OUT_PS_VBIAS_DDR  SBIT(DDRD,  4)

#define OUT_SDRAM_CLKE      SBIT(PORTC, 5)  /* Port C pin 5, active high */
#define OUT_SDRAM_CLKE_DDR  SBIT(DDRC,  5)
#define OUT_BOOT_ACK        SBIT(PORTC, 6)  /* Port C pin 6, active high */
#define OUT_BOOT_ACK_DDR    SBIT(DDRC,  6)
#define OUT_UART_EXT        SBIT(PORTC, 7)  /* Port C pin 7, active high */
#define OUT_UART_EXT_DDR    SBIT(DDRC,  7)

#define SPI_MOSI      SBIT(PORTB, 5)  /* Port B pin 5, Master out */
#define SPI_MOSI_DDR  SBIT(DDRB,  5)
#define SPI_CLK       SBIT(PORTB, 7)  /* Port B pin 7, SPI clock output */
#define SPI_CLK_DDR   SBIT(DDRB,  7)

#elif defined SUSKA_III_B

/* Input port pins */
#define IN_RESET_POWER    SBIT(PIND,  5)  /* - OK - Port D pin 5, active high */
#define IN_RESET_CORE     SBIT(PIND,  4)  /* - OK - Port D pin 4, active high */
#define IN_BOOT_REQ       SBIT(PIND,  7)  /* - OK - Port D pin 7, active high */
#define SPI_MISO          SBIT(PINB,  6)  /* - OK - Port B pin 6, Master in */
#define FPGA_CONFIG_DONE  SBIT(PINA,  1)  /* - OK - Port A pin 1, FPGA configuration done, active high */

/* Output port pins */
#define OUT_RESET           SBIT(PORTC, 1)  /* - OK - Port C pin 1, active high */
#define OUT_RESET_DDR       SBIT(DDRC,  1)
#define OUT_RESET_CORE      SBIT(PORTC, 0)  /* - OK - Port C pin 0, active high */
#define OUT_RESET_CORE_DDR  SBIT(DDRC,  0)

#define OUT_PS_EN         SBIT(PORTD, 6)  /* - OK - Port D pin 6, active high */
#define OUT_PS_EN_DDR     SBIT(DDRD,  6)

#define OUT_BOOT_ACK        SBIT(PORTB, 4)  /* - OK - Port B pin 4, active high */
#define OUT_BOOT_ACK_DDR    SBIT(DDRB,  4)

#define SPI_MOSI      SBIT(PORTB, 5)  /* - OK - Port B pin 5, Master out */
#define SPI_MOSI_DDR  SBIT(DDRB,  5)
#define SPI_CLK       SBIT(PORTB, 7)  /* - OK - Port B pin 7, SPI clock output */
#define SPI_CLK_DDR   SBIT(DDRB,  7)

#endif

