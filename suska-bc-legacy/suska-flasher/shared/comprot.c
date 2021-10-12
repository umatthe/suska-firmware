/***
 * Program to receive and transmit a stream of characters over RS232/USB
 * The program secures the data transmission with REQ, ACK and a CRC 
 *
 * Copyright (c) 2007, Inventronik GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the Inventronik nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdint.h>
 
#include "../shared/comprot.h"
#include "../shared/uart.h"

//#define HOST_DBG

/* to hold config device data */
uint16_t rcv_id = 0, snd_id = 0, snd_last_req_id = 0, rcv_last_req_id = 0;
//uint16_t rcv_id = 191, snd_id = 191, snd_last_req_id = 191, rcv_last_req_id = 191;
//uint16_t rcv_id = 238, snd_id = 238, snd_last_req_id = 238, rcv_last_req_id = 238;
char comprot_err[80] = {0}; /* error string */

/* cmd callback array */
static dc_callback_t dc_cb[] = {
	{CMD_SYNC, NULL},
	{CMD_WRITE, NULL},
	{CMD_READ, NULL},
	{CMD_CTL, NULL},
	{CMD_DBG, NULL}
};


/* -- Transfer functions to guarantee architecture independant byte order (network byte order) ---------- */
/**
 * host to netword byte order (network byte order is big endian)
 */
uint16_t htons(const uint16_t data)
{
	uint16_t one=1, tmp=data;

	if (*((uint8_t *)&one)) { /* are we little endian? */
		uint8_t *data_p = (uint8_t *) &data;
		tmp = (uint16_t) (data_p[0]<<8 | data_p[1]);
	}
	return tmp;
}
#define ntohs(a) htons(a)

uint32_t htonl(const uint32_t data)
{
	uint32_t one=1L, tmp=data;

	if (*((uint8_t *)&one)) { /* are we little endian? */
		uint8_t *data_p = (uint8_t *) &data;
		tmp = (uint32_t)((uint32_t)data_p[0]<<24 | (uint32_t)data_p[1]<<16 | (uint32_t)data_p[2]<<8 | (uint32_t)data_p[3]);
	}
	return tmp;
}
#define ntohl(a) htonl(a)

/**
 *                                       16   12   5
 * This is the CCITT-CRC-16 polynomial X  + X  + X  + 1.
 * This is 0x1021 when x is 2, but the way the algorithm works
 * we use 0x8408 (the reverse of the bit pattern). The high
 * bit is always assumed to be set, thus we only use 16 bits to
 * represent the 17 bit value.
 *
 * start with crc = 0xffff
 */
#define POLY 0x8408   /* 1021H bit reversed */
static uint16_t crc16(uint8_t *data_p, uint16_t length, uint16_t crc)
{
	uint8_t i;
	uint16_t data; /*, crc = 0xffff; */

	if (length == 0)
		return ~crc;
	do
	{
		for (i=0, data=(uint16_t)0xff & *data_p++; i < 8; i++, data >>= 1) {
			if ((crc & 0x0001) ^ (data & 0x0001))
				crc = (crc >> 1) ^ POLY;
			else  
				crc >>= 1;
		}
	} while (--length);

	crc = ~crc;
	data = crc;
	crc = (crc << 8) | (data >> 8 & 0xff);

	return crc;
}

/**
 * Return a checksum to verify data frame
 */
uint16_t create_crc(data_container_t *dc)
{
	uint16_t crc = 0xffff;
	uint8_t ident = IDENT_CODE;

	crc = crc16(&ident, sizeof(uint8_t), crc);
	crc = crc16((uint8_t *)dc, 2*sizeof(uint8_t)+2*sizeof(uint16_t), crc);

	/* calc crc for the payload */
	if (dc->size>0) {
		crc = crc16(dc->data, dc->size, crc);
	}
	return crc;
}

/**
 * Return printable character (hex dump helper)
 */
char print_c(char c)
{
	return (c<32 || c>126) ? '.' : c;
}

/**
 * Each ident code in this data stream has been doubled. This function discards
 * each double send ident codes in this data array.
 */
int8_t uart_receive_discard_ident(uint8_t *data, uint16_t size)
{
	uint8_t c;
	uint16_t i;

	for (i=0; i<size; i++) {
		if (uart_receive(&c, 1)<0)
			return ERR_TIMEOUT; /* timeout */
		data[i] = c;

		/* discard 2nd ident code */
		if (c==IDENT_CODE) {
			if (uart_receive(&c, 1)<0)
				return ERR_TIMEOUT; /* timeout */
		}
	}
	return 0;
}

/**
 * Uart send doubles ident codes in data
 */
int8_t uart_send_double_ident(uint8_t *data, uint16_t size)
{
	uint16_t i=0, j=0;
	uint8_t *data_p=data, c=IDENT_CODE;

	do {
		while (i<size && *data_p!=IDENT_CODE) {
			i++; data_p++;
		} 

		uart_send(data, i-j);
		if (*data_p++==IDENT_CODE) {
			uart_send(&c, sizeof(uint8_t));
			uart_send(&c, sizeof(uint8_t));
			i++;
			j = i;
		}
		data = data_p;
	} while (i<size);
	return 0;
}


/* -- Receive and check functions ------------------------------------------------------------------------------------------ */
/**
 * Receive a whole frame of data (and check borders and timeouts)
 */
int8_t recv_and_check_dc(data_container_t *dc, uint16_t max_payload_size)
{
	uint16_t hsize, crc;
	int8_t err;
	uint8_t *data_p = (uint8_t *)dc, c, cn;

	/* wait for single ident code */
	do {
		if (uart_receive(&c, sizeof(uint8_t))<0)
			return ERR_TIMEOUT; /* timeout */

		/* next character (cn) must not be an ident code! */
		if (uart_receive(&cn, sizeof(uint8_t))<0)
			return ERR_TIMEOUT; /* timeout */

	} while (c!=IDENT_CODE || cn==IDENT_CODE);

#ifdef HOST_DBG
#ifndef __AVR__
	fprintf(stderr, "***Receive: got ident: 0x%02x and cmd: 0x%02x\n", c, cn);
#endif
#endif

	/* header size: leave away data pointer and crc 
	   as well as cmd which we got through cn */
	hsize = sizeof(data_container_t) - sizeof(uint8_t *) - sizeof(uint16_t) - sizeof(uint8_t);

	/* lets recv the header (cmd, type, id, size) of a package */
	*data_p++ = cn; /* cmd we got already */
	if ((err=uart_receive_discard_ident(data_p, hsize))<0)
		return err;
#ifdef HOST_DBG
#ifndef __AVR__
	{ int16_t i; for (i=-1; i<hsize; i++) printf("h%02d: 0x%02x\n", i+1, data_p[i]); }
#endif
#endif

	/* network byte order endianess */
	dc->size = ntohs(dc->size);
	dc->id = ntohs(dc->id);

	/* how much data to come? */
	if (dc->size>max_payload_size) {
#ifdef HOST_DBG
#ifndef __AVR__
	printf("dc->size: 0x%04x > max_payload_size: 0x%04x\n", dc->size, max_payload_size);
#endif
#endif
		return ERR_NOT_A_VALID_SIZE; /* not a valid size */
	}
	/* save request id to send ack */
	if (dc->type==TYPE_REQ)
		rcv_last_req_id = dc->id;

#ifdef HOST_DBG
#ifndef __AVR__
	if (dc->size>0) 
		printf("\n***** Receive payload: %d\n", dc->size);
	else
		printf("\n***** No payload\n");
#endif
#endif
	/* lets recv payload */
	if (dc->size>0 && (err=uart_receive_discard_ident(dc->data, dc->size))<0)
		return err;
#ifdef HOST_DBG
#ifndef __AVR__
	{
		uint16_t i;
		for (i=0; i<dc->size; i+=8)
			printf("*** 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x %c%c%c%c%c%c%c%c\n", dc->data[i], dc->data[i+1], dc->data[i+2], dc->data[i+3], dc->data[i+4], dc->data[i+5], dc->data[i+6], dc->data[i+7], print_c(dc->data[i]), print_c(dc->data[i+1]), print_c(dc->data[i+2]), print_c(dc->data[i+3]), print_c(dc->data[i+4]), print_c(dc->data[i+5]), print_c(dc->data[i+6]), print_c(dc->data[i+7]));
	}
#endif
#endif

	/* last but not least, recv the 16bit-crc */
	if ((err=uart_receive_discard_ident((uint8_t *) &crc, sizeof(uint16_t)))<0)
		return err; /* timeout */
	dc->crc = ntohs(crc);

	/* lets check the crc */
#ifdef HOST_DBG
#ifndef __AVR__
	printf("crc ok?: 0x%04x -> 0x%04x (size: %d)\n", create_crc(dc), dc->crc, dc->size);
	show_dbg(dc);
#endif
#endif
	if (dc->crc!=create_crc(dc))
		return ERR_WRONG_CRC; /* wrong crc */

	/* All Ok so far! Let's exec callback if registered */
	c=0; 
	while (c<sizeof(dc_cb)/sizeof(dc_callback_t)) {
		if (dc_cb[c].ptr2func!=NULL && dc->cmd==dc_cb[c].cmd) {
			ptr2func_t func = dc_cb[c].ptr2func;
			(*func)(dc); /* exec callback */
			break;
		}
		c++;
	}
	rcv_id++; /* incr. id counter */
	return 0;
}


/**
 * Wait for a specific dc and handle all DBG messages in between
 * we expect that a callback function for DBG messages has been established
 */
int8_t wait_for_dc(data_container_t *dc, uint8_t cmd, uint8_t type, uint16_t max_payload_size)
{
	int8_t retc, attempts=0;

	comprot_err[0] = '\0';
	while ((retc=recv_and_check_dc(dc, max_payload_size))>=ERR_TIMEOUT && attempts<RETRY_ATTEMPTS) {
		/* ignore timeouts and dbg msgs */
		if (retc==0) { /* no error */
			if (dc->cmd==CMD_DBG)
				continue;
			if (dc->type==TYPE_NACK) {
				return ERR_NACK;
			}
			if (!(dc->cmd & cmd)) {
				return ERR_UNEXPECTED_CMD;
			}
			if (dc->type!=type) {
				return ERR_UNEXPECTED_TYPE;
			}
			if (dc->type==TYPE_ACK && dc->id!=snd_last_req_id) {
				return ERR_WRONG_ID; /* we are missing some data! */
			}
			return 0; /* we got what we wanted! */
		}
		attempts++;
	}
	if (attempts>=RETRY_ATTEMPTS)
		retc = ERR_TIMEOUT;
	return retc; /* return first grade error */
}


/* -- Send and create functions --------------------------------------------------------------------------- */

/**
 * Create and send an data container
 */
void create_and_send_dc(uint8_t cmd, uint8_t type, uint16_t size, uint8_t *data)
{
	uint8_t c;
	uint16_t crc, hsize;
	data_container_t dc;

#ifdef HOST_DBG
#ifndef __AVR__
	printf("***Send:\n");
#endif
#endif
	dc.cmd = cmd;
	dc.type = type;
	dc.id = ((type==TYPE_ACK) ? rcv_last_req_id : rcv_id);
	if (type==TYPE_REQ) {
		snd_last_req_id = dc.id;
	}
	dc.size = size;
	dc.data = data;

	/* create crc with raw data (not in network byte order) */
	crc = htons(create_crc(&dc));

	dc.id   = htons(dc.id);
	dc.size = htons(dc.size);

	/* leave away data pointer and crc */
	hsize = sizeof(data_container_t) - sizeof(uint8_t *) - sizeof(uint16_t);
#ifdef HOST_DBG
#ifndef __AVR__
	{ int16_t i; uint8_t *data_p = (uint8_t *)&dc; for (i=0; i<hsize; i++) printf("h%02d: 0x%02x\n", i, data_p[i]); }
#endif
#endif

	/* send ident code */
	c = IDENT_CODE;
	uart_send(&c, sizeof(uint8_t));

	/* send header */
	uart_send_double_ident((uint8_t *) &dc, hsize);

	/* send payload */
	if (size>0)
		uart_send_double_ident(data, size);

	/* send crc */
	uart_send_double_ident((uint8_t *) &crc, sizeof(uint16_t));
	snd_id++;
}

/**
 * let's send a debug message to the receiver.
 */
void send_dbg(char *str, ...)
{
	char strdata[60];
	va_list ap;

	va_start(ap, str);
	vsnprintf(strdata, 60, str, ap);
	va_end(ap);

// there must be a bug!!! avr crashes if create_and_send_dc !!!!!!!!!!!!!!!!
#ifndef __AVR__
	create_and_send_dc(CMD_DBG, TYPE_REQ, strlen(strdata), (uint8_t *)strdata);
#endif
}

/**
 * Add a function pointer for each cmd
 */
int8_t add_cmd_callback(uint8_t cmd, ptr2func_t ptr2func)
{
	uint8_t i, retval =- 1;

	for (i=0; i<sizeof(dc_cb)/sizeof(dc_callback_t); i++) {
		if (dc_cb[i].cmd==cmd) {
			dc_cb[i].ptr2func = ptr2func;
			retval = 0;
		}
	}
	return retval;
}

/* -- Callbacks ------------------------------------------------------------------------------------------------------------ */
/**
 * Callback to print a clients debug message (host only)
 */
void show_dbg(data_container_t *dc)
{
	printf("--recv'd a valid command--\n");
	printf("cmd:  0x%02x\n", dc->cmd);
	printf("type: 0x%02x\n", dc->type);
	printf("id:   %d\n", dc->id);
	printf("size: %d\n", dc->size);
	printf("crc:  0x%02x\n", dc->crc);
	if (dc->cmd==CMD_DBG)
		printf("str:  '%s'\n", dc->data);
}

/**
 * Callback send an ACK to a SYNC request (client only)
 */
void ack_sync(data_container_t *dc)
{
	create_and_send_dc(CMD_SYNC, TYPE_ACK, 0, NULL);
}

/**
 * Callback send an ACK to a DATA request (client only)
 */
void ack_data(data_container_t *dc)
{
	create_and_send_dc(CMD_DATA, TYPE_ACK, 0, NULL);
}

