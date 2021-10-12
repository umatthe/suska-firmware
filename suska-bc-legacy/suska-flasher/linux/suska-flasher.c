/*
 * This is a simple communiction program that uses a USB/serial to
 * send a binary file from a PC (host) to a microcontroller (client)
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
 * (c) 2007, 2008 by Inventronik GmbH, Jens Carroll
 */
#include <stdio.h>	   /* for printf */
#include <stdlib.h>    /* for exit */
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../shared/comprot.h"

//#define DEBUG

#ifndef TRUE
#define FALSE 0
#define TRUE !FALSE
#endif

/* tty stuff */
static int sfd = -1;
static struct termios old_termios;

/* print additional info */
static int verbose = FALSE;
static int flash_addr = 0;
static char flash_erase = TRUE;

/* dc receiving payload buffer */
static uint8_t rcvbuf[PKG_SIZE];

#define MAX_FILE_SIZE	(8*1024*1024) /* Flash size */
#define MAX_TIMEOUT_COUNT	2500
#define MAX_SYNC_TRY		10


/* Error descriptions */
err_t err_type[] = {
	{ERR_TIMEOUT,          "Timeout occured"},
	{ERR_WRONG_IDENT,      "Wrong ident code received (!=0x1a)"},
	{ERR_WRONG_ID,         "Wrong id"},
	{ERR_NOT_A_VALID_SIZE, "Not a valid size"},
	{ERR_WRONG_CRC,        "Wrong CRC"},
	{ERR_NACK,             "NACK received"},
	{ERR_UNEXPECTED_CMD,   "Unknown or unexpected command"},
	{ERR_UNEXPECTED_TYPE,  "Unknown or unexpected type"},
	{0, NULL} /* marks the last entry */
};

/* Prototypes */
static void close_port(void);
static int open_port(const char *device, int baudrate);
static int write_port(const char *data, int len);
static int read_port(char *data, int len);
static void update_progress(int percent, double etime, char *hdr);
static void signal_handler(int sig);
static void usage(char *name);
static void verbose_printf(int prog_exit, char *str, ...);

/* - error handling functions ------------------------------------------------------------- */
static char *get_err_msg(int8_t err_no)
{
	err_t *err_ptr = err_type;

	while (err_ptr->err_no!=err_no && err_ptr->err_no!=0)
		err_ptr++;
	return err_ptr->err_msg;
}

/**
 * Additional output for debugging, with an exit possibility 
 */
static void verbose_printf(int prog_exit, char *str, ...)
{
    va_list argptr;
    char strdata[256];

	if (verbose) {
		va_start(argptr, str);
		vsnprintf(strdata, 256, str, argptr);
		va_end(argptr);
		fprintf(stderr, "%s", strdata);
	}
	if (prog_exit)
		exit(EXIT_FAILURE);
}

/**
 * Delay in milliseconds
 */
void delay_ms(int delay)
{
	usleep(delay*1000);
}

/* - low-level serial functions ----------------------------------------------------------- */
/**
 * opens the serial port
 * return code:
 *	 > 0 = fd for the port
 *	 -1 = open failed
 */
static int open_port(const char *device, int baudrate)
{
	/* make sure port is closed */
	close_port();

	if ((sfd = open(device, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
		verbose_printf(TRUE, "Open port error: %s (%d)\n", strerror(errno), errno);
	} else {
		int baud;
		struct termios my_termios;

		tcgetattr(sfd, &old_termios);

		switch(baudrate) {
			case 9600: 
				baud = B9600; 
				break;
			case 19200: 
				baud = B19200; 
				break;
			case 57600: 
				baud = B57600; 
				break;
			case 115200:
				baud = B115200;
				break;
			case 38400: 
			default:
				baud = B38400; 
		}

		bzero(&my_termios, sizeof(my_termios));
		my_termios.c_cflag = baud | CS8 | CREAD | CLOCAL | HUPCL;
		my_termios.c_iflag = IGNPAR;
		my_termios.c_oflag = 0;

		/* set input mode (non-canonical, no echo,...) */
		my_termios.c_lflag = 0;
		cfsetospeed(&my_termios, baud);

		tcflush(sfd, TCIFLUSH);
		tcsetattr(sfd, TCSANOW, &my_termios);
	}
	return sfd;
}

/**
 * closes the serial port
 */
static void close_port(void)
{
	if (sfd > 0) {
		tcsetattr(sfd, TCSANOW, &old_termios);
		close(sfd);
	}
}

/**
 * writes length number of bytes to the serial port
 * return code:
 *	>= 0 = number of characters written
 *	-1 = write failed
 */
static int write_port(const char *data, int len)
{
	int out, remain, timeout = 0;

	if (sfd < 1) {
		verbose_printf(TRUE, "Port is not open for writing\n");
	}

	remain = len;
	do {
		if (timeout++)
			usleep(1000);
		if ((out=write(sfd, &data[len-remain], remain)) > 0) {
			remain -= out;
		}
#ifdef DEBUG
		if (out>0) {
			printf("---> send %d (of %d) chars (0x%02x = '%c')\n", len-remain, len, (unsigned char)*data, (char)*data);
			int i;
			for(i=0; i<len-remain; i++)
				printf("0x%02x ", (unsigned char) data[i]);
			printf("\n\n");
		}
#endif
	} while ((remain>0 && timeout<MAX_TIMEOUT_COUNT) || (out==-1 && errno==EAGAIN));

	if (timeout>=MAX_TIMEOUT_COUNT) {
		verbose_printf(TRUE, "\nWrite error (%d)\n  %s\n", errno, strerror(errno));
	}
	return out;
}

/**
 * read max. length number of bytes from the serial port
 * return code:
 *	>= 0 = number of characters read
 *	-1 = read failed
 */
static int read_port(char *data, int len)
{
	int in, remain, timeout = 0;

	if (sfd < 1) {
		verbose_printf(TRUE, "Port is not open for reading\n");
	}

	remain = len;
	do {
		if (timeout++)
			usleep(1000); /* 1ms */
		if ((in=read(sfd, &data[len-remain], remain)) > 0) {
			remain -= in;
		}
	} while ((remain>0 && timeout<MAX_TIMEOUT_COUNT) || (in==-1 && errno==EAGAIN));
	
#ifdef DEBUG
	if (timeout>=MAX_TIMEOUT_COUNT) {
	printf("---> read %d (of %d) chars (0x%02x = '%c') tout=%d, errno=%d\n", len-remain, len, (unsigned char)*data, (char)*data, timeout, errno);
/*
	int i;
	printf(">>recvd>>");
	if (len==6 || len==4) {
		for(i=0; i<len-remain; i++)
			printf("0x%02x ", (unsigned char) data[i]);
	} else {
		for(i=0; i<len; i++)
			printf("%c", data[i]);
	}
	printf("\n\n");
*/
	}
#endif
	return (timeout>=MAX_TIMEOUT_COUNT ? -1 : len); 
}

/**
 * Add a progress bar to the output
 */
static void update_progress(int percent, double etime, char *hdr)
{
#ifndef DEBUG
	static char hashes[51];
	int i;

	hashes[50] = 0;

	memset(hashes, ' ', 50);
	for (i=0; i<percent; i+=2) {
		hashes[i/2] = '#';
	}
	fprintf(stderr, "\r%s | %s | %d%% %0.2fs", hdr, hashes, percent, etime);

	if (percent == 100) {
		fprintf(stderr, "\n\n");
	}
#endif
}



/* - uart interface for comprot.c --------------------------------------------------------- */
int8_t uart_send(uint8_t *cptr, uint16_t size)
{
	write_port((char *)cptr, size);
	return 0; /* timeout will cause a program exit, 0 indicates success */
}

int8_t uart_receive(uint8_t *cptr, uint16_t size)
{
	return (int8_t) read_port((char *)cptr, size);
}

/* - Additional functions  ---------------------------------------------------------------- */
/**
 * Read data from file
 * Param: filename, pointer to the data (unsigned char) and
 * the size of the data array
 */
int read_file(const char *fname, unsigned char **dptr, unsigned int size)
{
	FILE *in;

	/* open the file we are reading from */
	if (!(in = fopen(fname, "r"))) {
		verbose_printf(TRUE, "Can't open file '%s'\n", fname);
	}

	/* allocate max. 8MB */
	*dptr = (unsigned char *) calloc(size, sizeof(unsigned char));
	if (*dptr==NULL) {
		fclose(in);
		verbose_printf(TRUE, "Can't allocate memory (%d Bytes)\n", size);
	} else {
		verbose_printf(FALSE, "Allocated memory (%d Bytes)\n", size);
	}

	/* fread to read binary data from file */
	size = fread(*dptr, sizeof(unsigned char), size, in);
	verbose_printf(FALSE, "File '%s' read. Size: %d\n", fname, size);
	fclose(in);

	return size;
}

/**
 * Write data to file
 * Param: filename, pointer to the data (unsigned char) and
 * the size of the data array
 */
int write_file(const char *fname, unsigned char *dptr, int size)
{
	FILE *out;

	/* open the file we are writing to */
	if (!(out = fopen(fname, "w"))) {
		verbose_printf(TRUE, "Can't open file %s\n", fname);
	}

	/* fwrite to write binary data to the file */
	size = fwrite(dptr, sizeof(unsigned char), size, out);
	verbose_printf(FALSE, "Write file '%s'. Size: %d\n", fname, size);

	fclose(out);
	return size;
}

/* ----------------------------------------------------------------------------------- */
/**
 * Since the HOST communication program will be started first, we try
 * to get in sync with the microcontroller.
 */
void sync_and_wait_ack(void)
{
	int i=0, stat;
	data_container_t dc;
	dc.data = rcvbuf;

	verbose_printf(FALSE, "Try to SYNC with mcu ");

	while (i<MAX_SYNC_TRY) {
		/* send sync cmd to the mcu */
		create_and_send_dc(CMD_SYNC, TYPE_REQ, 0, NULL);
		stat = recv_and_check_dc(&dc, PKG_SIZE);

		if (stat < 0) {
			if (stat==ERR_TIMEOUT) {
				verbose_printf(FALSE, "%d.", i+1); /* Timeout */
				i++;
			} else {
				verbose_printf(TRUE, "Got invalid answer from mcu: err: %d -> %s\n", stat, get_err_msg(stat));
			}
		} else if (dc.type==TYPE_ACK) {
			if (dc.cmd==CMD_SYNC) {
				break; /* got sync */
			}
		} else {
			verbose_printf(FALSE, "?"); /* we should not receive anything else as TYPE_ACK */
		}
	}
	verbose_printf(FALSE, "\n");

	if (i<MAX_SYNC_TRY) {
		verbose_printf(FALSE, "Got SYNC from mcu\n");
	} else {
		fprintf(stderr, "No response from mcu!\n");
		exit(EXIT_FAILURE);
	}
}


/**
 * Send a file to the client
 */
static int write_to_client(const char *fname)
{
	unsigned char data[PKG_SIZE], *fbuff, cmd;
	int fsize, pages, left, i, err = 0;
	char *hdr = "Writing: ";
	struct timeval tv;
	double t, time_start, pstep;
	data_container_t dc;
	dc.data = rcvbuf;

	/* first read the whole file */
	fsize = read_file(fname, &fbuff, MAX_FILE_SIZE);

	pages = fsize / PKG_SIZE;
	left = fsize % PKG_SIZE;
	pstep = 100.0/(pages+1);

	/* send write request (doesn't include filesize) */
	create_and_send_dc(CMD_WRITE, TYPE_REQ, 0, NULL);
	if ((err = wait_for_dc(&dc, CMD_WRITE, TYPE_ACK, PKG_SIZE)) < 0) {
		fprintf(stderr, "\n*Got invalid answer from mcu: err: %d -> %s\n", err, get_err_msg(err));
		goto done;
	}

	/* wait for the flash to be erased */
	if (flash_erase) {
		verbose_printf(FALSE, "Wait for Flash to be erased (about 60 secs)\n");
		if (!verbose) {
			printf("Erasing the Flash (it takes about 60 secs) ...\n");
		}
		delay_ms(60000);
	}

	/* save start time */
	gettimeofday(&tv, NULL);
	time_start = tv.tv_sec + ((double)tv.tv_usec)/1000000;

	/* next send all data */
	for (i=0; i<pages; i++) {
		memcpy(data, &fbuff[i*PKG_SIZE], PKG_SIZE);

		/* last package empty? */
		cmd = (left==0 && i==pages-1) ? CMD_DATA_LAST : CMD_DATA;
		create_and_send_dc(cmd, TYPE_REQ, PKG_SIZE, data);

		gettimeofday(&tv, NULL);
		t = tv.tv_sec + ((double)tv.tv_usec)/1000000;
		update_progress(cmd==CMD_DATA_LAST ? 100 : ((int)(pstep*(i+1))), t-time_start, hdr);

		if ((err = wait_for_dc(&dc, cmd, TYPE_ACK, PKG_SIZE)) < 0) {
			fprintf(stderr, "\n**Got invalid answer from mcu: err: %d -> %s\n", err, get_err_msg(err));
			goto done;
		} else {
#ifdef DEBUG
			show_dbg(&dc);
#endif
		}
	}
	/* send last package */
	if (left) {
		memcpy(data, &fbuff[i*PKG_SIZE], left);
		create_and_send_dc(CMD_DATA_LAST, TYPE_REQ, left, data);

		if((err = wait_for_dc(&dc, CMD_DATA_LAST, TYPE_ACK, PKG_SIZE)) < 0) {
			fprintf(stderr, "\n***Got invalid answer from mcu: err: %d -> %s\n", err, get_err_msg(err));
			goto done;
		} else {
#ifdef DEBUG
			show_dbg(&dc);
#endif
		}
		gettimeofday(&tv, NULL);
		t = tv.tv_sec + ((double)tv.tv_usec)/1000000;
		update_progress(100, t-time_start, hdr);
	}

done:
	verbose_printf(FALSE, "Free memory: %d Bytes\n", MAX_FILE_SIZE);
	free(fbuff);
	return err;
}

/**
 * Take action and check for flash mode, eventually send flash start address
 */
int prepare_for_flash_mode(void)
{
	int err, wait, result = TRUE;
	data_container_t dc;
	dc.data = rcvbuf;

	/* has a FLASH addr been specified? */
	if (flash_addr!=0) {
		char addrstr[20];

		verbose_printf(FALSE, "Set flash start address to: 0x%x\n", flash_addr);
		sprintf(addrstr, "A%x", flash_addr>>8);
		create_and_send_dc(CMD_CTL, TYPE_REQ, strlen(addrstr)+1, (uint8_t *)addrstr);

		if ((err = wait_for_dc(&dc, CMD_CTL, TYPE_ACK, PKG_SIZE)) < 0) {
			verbose_printf(TRUE, "\n****Got invalid answer from mcu: err: %d -> %s\n", err, get_err_msg(err));
			result = FALSE;
			goto done;
		} else {
			verbose_printf(FALSE, "Address accepted: %s\n", dc.data[0]=='A' ? "OK" : "failed");
		}
	}

	/* set mode - don't erase flash */
	verbose_printf(FALSE, "Preserve flash content: ");
	create_and_send_dc(CMD_CTL, TYPE_REQ, 2, (uint8_t *)(flash_erase ? "Me" : "Mn"));

	if ((err = wait_for_dc(&dc, CMD_CTL, TYPE_ACK, PKG_SIZE)) < 0) {
		verbose_printf(TRUE, "\n****Got invalid answer from mcu: err: %d -> %s\n", err, get_err_msg(err));
		result = FALSE;
		goto done;
	} else {
		verbose_printf(FALSE, (flash_erase ? "No\n" : "Yes\n"));
	}

	/* synchronize with the FPGA */
	do {
		static int first_msg = TRUE;
		wait = TRUE;
		create_and_send_dc(CMD_CTL, TYPE_REQ, 1, (uint8_t *)"W");
		if ((err = wait_for_dc(&dc, CMD_CTL, TYPE_ACK, PKG_SIZE)) < 0) {
			verbose_printf(TRUE, "\n*****Got invalid answer from mcu: err: %d -> %s\n", err, get_err_msg(err));
			result = FALSE;
		} else {
			if (dc.size == 2) {
				wait = (dc.data[0]=='W' && dc.data[1]=='r') ? FALSE : TRUE;
			}
			if (wait) {
				if (first_msg) {
					verbose_printf(FALSE, "w - waiting for Suska to be in Flash programming mode!\n");
					first_msg = FALSE;
				}
				verbose_printf(FALSE, "w");
				delay_ms(1000);
			}
		}
	} while (wait);

done:
	verbose_printf(FALSE, "\n");
	return result;
}


/**
 * fpga-prog usage
 */
static void usage(char *name)
{
	printf("usage: %s [-h] [-v] [-n] [-s serial port] [-b baudrate] [-a flash address] filename\n", name);
	printf("      -h: this help\n");
	printf("      -a: address   - 24 bit hex start address (e.g. 0x100000, 0x180000)\n");
	printf("      -b: baudrate  - 9600, 19200, 38400, 57600, 115200 default: 38400\n"); 
	printf("      -n            - don't erase FLASH-Prom\n"); 
	printf("      -s: port      - serial port (default: /dev/ttyUSB0)\n");
	printf("      -v            - (verbose) additional debugging infos\n");
	printf("          filename  - file to write\n\n");
	printf("examples: '%s -b 19200 tos204.img'\n", name);
	printf("          programming of the Suska FLASH-Prom with a baudrate of 38400bps.\n");

	exit(EXIT_FAILURE);
}

/* - Signal handling --------------------------------------------------------------------- */
/**
 * signal handler
 */
void signal_handler(int sig)
{
	switch (sig) {
		case SIGHUP:
		case SIGTERM:
		case SIGINT:
			fprintf(stderr, "Signal handler caught (%d)\n", sig);
		break;
	}
}

/**
 * main routine <param> (see usage)
 */
int main(int argc, char *argv[])
{
	char device[256] = "/dev/ttyUSB0";
	char fname[128];
	int baudrate = 38400;
	int all_ok = -1;

	/* check aguments */
	while (1) {
		int c = getopt(argc, argv, "?ha:b:s:vdn");
		if (c == -1)
			break;

		switch (c) {
			case '?': /* help */
			case 'h':
				usage(argv[0]);
			case 'a': /* flash address */
				if (!strncmp(optarg, "0x", 2) && strlen(optarg)==8) {
					flash_addr = strtol(optarg+2, NULL, 16);
				}
				else
					usage(argv[0]);
				break;
			case 'b': /* set baudrate */
				baudrate = atoi(optarg);
				break;
			case 'n': /* do not erase flash prom */
				flash_erase = FALSE;
				break;
			case 's': /* set tty device */
				strncpy(device, optarg, 256);
				break;
			case 'v': /* verbose */
				verbose = TRUE;
				break;
		}
	}

	if (argc == 1 || optind >= argc) {
		usage(argv[0]);
	} else {
		strncpy(fname, argv[optind], 127);
		fname[127] = '\0';
	}

	if (!verbose) {
		printf("Programming Flash, please wait ...\n");
	}

	/* first open the serial port */
	open_port(device, baudrate);

	/* establish signal handlers */
  signal(SIGHUP, signal_handler);  /* catch hangup signal */
	signal(SIGTERM, signal_handler); /* catch kill signal */
	signal(SIGUSR1, signal_handler); /* catch usr1 signal */
	signal(SIGUSR2, signal_handler); /* catch usr2 signal */
	signal(SIGCHLD, signal_handler); /* catch chld signal */

	/* print DBG message through callback */
	add_cmd_callback(CMD_DBG, show_dbg);

	/* let's get in sync with the microctrl */
	sync_and_wait_ack();

	/* prepare FLASH mode */
	verbose_printf(FALSE, "Prepare for programming the Flash PROM\n");
	if (prepare_for_flash_mode()) {
		/* start our work */
		all_ok = write_to_client(fname);
	}

	if (all_ok==0)
		printf("Flash programed successfully\n");
	else
		printf("Flash programming failed!\n");
	close_port();
	return EXIT_SUCCESS;
}
