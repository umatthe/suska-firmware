#ifndef _UART_H_
#define _UART_H_

void uart_init(uint16_t baudrate);
int8_t uart_send(const uint8_t *, uint16_t);
int8_t uart_receive(uint8_t *, uint16_t);
void uart_set_timeout(uint16_t);
int8_t uart_putc(const uint8_t);
#define uputch uart_putc

#endif /* _UART_H_ */
