/**
 * This implements a simple communiction protocol
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
 * used to endorse or promote products derived from this software without 
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * (c) 2007 by Inventronik GmbH, Dipl.-Ing. Jens Carroll
 */

/**
 * Avoiding any serial data transfer problems, we are using a simple
 * protocol. Each package starts with an ID (0x55) followed by a command.
 * A count-id follows. The counter needs to be increased by any package sent.
 * Depending on the command there will be a load that carries data. The
 * size parameter says how many bytes will follow. A size of 0 says, there
 * is no data load attached. A final crc has been build over all data that
 * has been sent.
 */

#ifndef _COMPROT
#define _COMPROT
#include <stdint.h>

#define IDENT_CODE		0x1a	/* each package starts with an package-id 0x1a */

/* DON't use the ident code above for CMD_, TYPE_ definitions */
#define CMD_SYNC		0x01	/* sync command will be send up-to 20 times to sync. */
#define CMD_CTL			0x02	/* send a cmd argument (application specific) */
#define CMD_WRITE		0x04	/* write command (write file/data to the client) */
#define CMD_READ		0x08	/* read command to request a file/data from the client */
#define CMD_DATA		0x10	/* a data package */
#define CMD_DATA_LAST	0x20	/* last data package */
#define CMD_DBG			0x40	/* this is a debug string ONLY send from the client no ACK necessary! */

#define TYPE_REQ		0x01	/* Request (read, write, sync) */
#define TYPE_ACK		0x02	/* ACK for each package sent */
#define TYPE_NACK		0x04	/* NACK in case of an error */

#define PKG_SIZE		256		/* max size of a data package (payload) */
#define TIMEOUT_MS		5000	/* Timeout before program aborts transmission */
#define RETRY_ATTEMPTS	10		/* max. number of retrys before giving up */

/* Error codes */
#define ERR_TIMEOUT					(-1)
#define ERR_WRONG_IDENT				(-2)
#define ERR_WRONG_ID				(-3)
#define ERR_NOT_A_VALID_SIZE		(-4)
#define ERR_WRONG_CRC				(-5)
#define ERR_NACK					(-6)
#define ERR_UNEXPECTED_CMD			(-7)
#define ERR_UNEXPECTED_TYPE			(-8)

/* typedef used for error descriptions */
typedef struct {
	int8_t err_no;
	char *err_msg;
} err_t;

/* structure to send via serial line (pack this struct byte aligned) */
typedef struct __attribute__ ((packed)) {
	uint8_t cmd;		/* commands see above */
	uint8_t type;		/* types see above */
	uint16_t id;		/* an id-counter that will be increased for each package */
	uint16_t size;		/* size of the data (0..PKG_SIZE) */
	uint8_t *data;		/* pointer to to the payload array */ 
	uint16_t crc;		/* a 16 bit checksum over all */
} data_container_t;

typedef	void (*ptr2func_t)(data_container_t *); /* typedef for function pointer */

typedef struct {
	uint8_t cmd;		 /* see above */
	ptr2func_t ptr2func; /* function pointer */
} dc_callback_t;

/* prototypes */
/*
uint16_t htons(const uint16_t data);
uint32_t htonl(const uint32_t data);
uint16_t ntohs(const uint16_t data);
uint32_t ntohl(const uint32_t data);
*/
uint16_t create_crc(data_container_t *dc);
int8_t recv_and_check_dc(data_container_t *dc, uint16_t max_payload_size);
int8_t wait_for_dc(data_container_t *dc, uint8_t cmd, uint8_t type, uint16_t max_payload_size);
void create_and_send_dc(uint8_t cmd, uint8_t type, uint16_t size, uint8_t *data);
void send_dbg(char *str, ...);
int8_t add_cmd_callback(uint8_t cmd, ptr2func_t ptr2func);
int8_t uart_receive_discard_ident(uint8_t *, uint16_t);
int8_t uart_send_double_ident(uint8_t *, uint16_t);
/* simple callbacks */
void show_dbg(data_container_t *dc);
void ack_sync(data_container_t *dc);

#endif
