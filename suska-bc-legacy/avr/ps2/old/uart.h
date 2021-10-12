#ifndef __UART_H_
#define __UART_H_

#define BAUD 7813
//#define BAUD 9600

#define XTAL F_CPU

//#define SYSCLK		XTAL
// Baudratenteiler so ausrechnen, dass der Fehler bedingt durch den
// gebrochenzahlige Rest kleiner als 0.2% ist
// Tabelle unter http://www.kreatives-chaos.com/index.php?seite=baud
// Bei gesetztem U2X gesetzt in UCSRA wird nicht /16UL sondern durch 8
// dividiert, aber die verdoppelte Baudrate angesetzt.

#define RING_SIZE 10
#define UBRR_BAUD ((XTAL / (16UL * BAUD)) - 1)
#define uputchar(x)	uputch(x)

void uart_init(void);
void uinit(void);
void uputch(char x);
void uputs(char *s);
void uputsnl(char *s);
void uputu(unsigned long d);
void uputd(long d);
void uputx(unsigned long d);
int  putchar(int c);
short int ugetchar(void);

#endif

char * itoadec(char * buf, int i, int len, char leading);
char * itoahex(char * buf, unsigned int i, int len, char leading);
