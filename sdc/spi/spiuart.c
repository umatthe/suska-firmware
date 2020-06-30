#include <avr/io.h>
#include "config.h"
#include "spiuart.h"

#ifdef SPIUART0
  #define XCKn_DDR XCK0_DDR
  #define XCKn XCK0
  #define UCSRnC UCSR0C
  #define UMSELn1 UMSEL01
  #define UMSELn0 UMSEL00
  #define UCPHAn UCPHA0
  #define UCPOLn UCPOL0
  #define UCSRnA UCSR0A
  #define UCSRnB UCSR0B
  #define RXENn RXEN0
  #define TXENn TXEN0
  #define UBRRn UBRR0
  #define UDREn UDRE0
  #define UDRn UDR0
  #define RXCn RXC0
#else
  #ifdef SPIUART1
    #define XCKn_DDR XCK1_DDR
    #define XCKn XCK1
    #define UCSRnC UCSR1C
    #define UMSELn1 UMSEL11
    #define UMSELn0 UMSEL10
    #define UCPHAn UCPHA1
    #define UCPOLn UCPOL1
    #define UCSRnA UCSR1A
    #define UCSRnB UCSR1B
    #define RXENn RXEN1
    #define TXENn TXEN1
    #define UBRRn UBRR1
    #define UDREn UDRE1
    #define UDRn UDR1
    #define RXCn RXC1
  #else
    #error "SPIUART Number not defined"
  #endif
#endif

void spiuart_init(uint8_t baud)
{
	UBRRn = 0;
	/* Setting the XCKn port pin as output, enables master mode. */
	XCKn_DDR |= (1<<XCKn);
	/* Set MSPI mode of operation and SPI data mode 0. */
	UCSRnC = (1<<UMSELn1)|(1<<UMSELn0)|(0<<UCPHAn)|(0<<UCPOLn);
	/* Enable receiver and transmitter. */
	UCSRnB = (1<<RXENn)|(1<<TXENn);
	/* Set baud rate. */
	/* IMPORTANT: The Baud Rate must be set after the transmitter is enabled
	*/
	UBRRn = baud;
}

uint8_t noinline spiuart_send(uint8_t data)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRnA & (1<<UDREn)) );
	/* Put data into buffer, sends the data */
	UDRn = data;
	/* Wait for data to be received */
	while ( !(UCSRnA & (1<<RXCn)) );
	/* Get and return received data from buffer */
	return UDRn;
}


#ifdef SPI_USE_CHUNK
void SpiWriteChunk(const uint8_t* pChunk, uint16_t nBytes)
{
        while (nBytes--)
	{
		/* Wait for empty transmit buffer */
		while ( !( UCSRnA & (1<<UDREn)) );
		/* Put data into buffer, sends the data */
		UDRn = *pChunk++;
		/* Wait for data to be received */
		while ( !(UCSRnA & (1<<RXCn)) );
	}
}

void SpiReadChunk(uint8_t* pChunk, uint16_t nBytes)
{
        while (nBytes--)
        {
		/* Wait for empty transmit buffer */
		while ( !( UCSRnA & (1<<UDREn)) );
		/* Put data into buffer, sends the data */
		UDRn = 0;
		/* Wait for data to be received */
		while ( !(UCSRnA & (1<<RXCn)) );
                *pChunk++ = UDRn;
        }

}


