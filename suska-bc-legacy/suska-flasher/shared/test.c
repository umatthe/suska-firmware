#include <stdio.h>
#include <stdlib.h>
#include "comprot.h"

uint8_t test1[] = {'a', 'b', IDENT_CODE, 'c', 'd' }; /* Err */
uint8_t test2[] = {'a', 'b', IDENT_CODE, IDENT_CODE, 'c', 'd' }; /* OK */
uint8_t test3[] = {'a', 'b', IDENT_CODE, 'c', IDENT_CODE, 'd' }; /* Err */
uint8_t test4[] = {IDENT_CODE, IDENT_CODE, 'a', 'b', 'c', IDENT_CODE, IDENT_CODE }; /* OK */
uint8_t test5[] = {IDENT_CODE, IDENT_CODE, 0xd3}; /* Err */
uint8_t test6[] = {IDENT_CODE, 0xd3}; /* Err */
uint8_t test7[] = {0xd3, IDENT_CODE}; /* Err */

uint8_t uart_fifo[256];
uint16_t uart_fifo_wptr=0, uart_fifo_rptr=0;
uint8_t uart_dbg=1;

void uart_clear_fifo(void)
{
	uart_fifo_rptr = uart_fifo_wptr = 0;
}

int8_t uart_receive(uint8_t *data, uint16_t size)
{
	uint16_t i;

	if (size+uart_fifo_rptr>256) {
		printf("uart receive: try to receive %d bytes, but available: %d\n", size, uart_fifo_rptr);
		exit(0);
	}

	if (uart_dbg)
		printf("*** Receive from uart\n");
	for (i=0; i<size && uart_fifo_rptr!=uart_fifo_wptr; i++) {
		data[i] = uart_fifo[uart_fifo_rptr+i];
		if (uart_dbg)
			printf("*** r: %d -> %c (0x%02x)\n", i, data[i], data[i]);
	}
	uart_fifo_rptr += size;
	if (uart_fifo_rptr==uart_fifo_wptr) {
		uart_fifo_rptr = uart_fifo_wptr = 0;
		if (uart_dbg)
			printf("!!!! *** rest rptr and wptr ***\n");
	}
			
	if (uart_dbg)
		printf("rptr: -> %d\n", uart_fifo_rptr);
	return 0;
}

int8_t uart_send(uint8_t *data, uint16_t size)
{
	uint16_t i;

	if(size+uart_fifo_wptr>256) {
		printf("uart send: try to send %d bytes, but fifo size: 256\n", size+uart_fifo_wptr);
		exit(0);
	}

	if (uart_dbg)
		printf("*** Send to uart\n");
	for (i=0; i<size; i++) {
		if (uart_dbg)
			printf("w: -> %d: 0x%02x (%c)\n", i, data[i], data[i]);
		uart_fifo[uart_fifo_wptr+i] = data[i];
	}
	uart_fifo_wptr += size;

	if (uart_dbg)
		printf("wptr: -> %d\n", uart_fifo_wptr);
	return 0;
}

int main(void)
{
	data_container_t dc;
	uint8_t data[256];

	dc.data = data;
	uint8_t *data_p;
	uint16_t crc;

	uart_send(test1, sizeof(test1));
	printf("Receive discard UART: Test1: %s\n--\n", uart_receive_discard_ident(uart_fifo, 4)==ERR_WRONG_IDENT ? "OK" : "Error");

	uart_clear_fifo();
	uart_send(test2, sizeof(test2));
	printf("Receive discard UART: Test2: %s\n--\n", uart_receive_discard_ident(uart_fifo, 5)==0 ? "OK" : "Error");

	uart_clear_fifo();
	uart_send(test3, sizeof(test3));
	printf("Receive discard UART: Test3: %s\n--\n", uart_receive_discard_ident(uart_fifo, 5)==ERR_WRONG_IDENT ? "OK" : "Error");

	uart_clear_fifo();
	uart_send(test4, sizeof(test4));
	printf("Receive discard UART: Test4: %s\n--\n", uart_receive_discard_ident(uart_fifo, 5)==0 ? "OK" : "Error");

	uart_clear_fifo();
	uart_send(test5, sizeof(test5));
	printf("Receive discard UART: Test5: %s\n--\n", uart_receive_discard_ident(uart_fifo, 2)==0 ? "OK" : "Error");

	uart_dbg=1;
	printf("Send double ident: Test1:\n--\n");
	uart_send_double_ident(test1, 5);

	printf("Send double ident: Test2:\n--\n");
	uart_send_double_ident(test2, 6);

	printf("Send double ident: Test3:\n--\n");
	uart_send_double_ident(test3, 0);

	printf("Send double ident: Test4:\n--\n");
	uart_send_double_ident(test4, 7);

	printf("Send double ident: Test5:\n--\n");
	uart_send_double_ident(test5, 2);


	uart_clear_fifo();
	data_p = (uint8_t *) &crc;
	*data_p++ = 0xd3;
	*data_p = 0x1a;
	printf("Test9 with crc:\n--\n");
	uart_send_double_ident((uint8_t *) &crc, sizeof(uint16_t));
	printf("End Test9 with crc:\n--\n");

//	uart_send(test6, sizeof(test6));
	printf("Test9-1: %s\n--\n", uart_receive_discard_ident(uart_fifo, 2)==0 ? "OK" : "Error");
#if 1
exit(0);
#endif

	/* print DBG message through callback */
	uart_dbg=1;
	add_cmd_callback(CMD_DBG, show_dbg);

	create_and_send_dc(CMD_DATA, TYPE_ACK, 0, data);
	wait_for_dc(&dc, CMD_READ, TYPE_REQ, 256);
	send_dbg("Hello world: %d", 1234);
	wait_for_dc(&dc, CMD_READ, TYPE_REQ, 256);
	exit(0);
}

