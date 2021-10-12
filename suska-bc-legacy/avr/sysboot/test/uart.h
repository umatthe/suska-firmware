#ifndef _UART_H_
#define _UART_H_

void uart_init (void);
int8_t uart_send(const uint8_t *, uint16_t);
int8_t uart_receive(uint8_t *, uint16_t);

#endif /* _UART_H_ */
