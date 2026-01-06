/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#include <fcntl.h>
#include <libftdi1/ftdi.h>
#include <linux/spi/spidev.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "args.h"
#include "chip_config.h"
#include "common.h"
#include "mpsse.h"
#include "rescue.h"
#include "shaft.h"
#include "transfer.h"

#define MAX_RETRIES_COUNTER 10

enum transfer_mode {
	TRANSFER_SPIDEV,
	TRANSFER_FTDI,
	TRANSFER_UART,
};

static struct {
	enum transfer_mode mode;
	struct mpsse_context *mpsse;
	char *name;
	int fd;
} xfer;

const uint32_t transfer_chunk_size = 1024 - offsetof(struct bootstrap_pkt, data);


/** 
 * Print a hex buffer to stdout.
 *
 * @param data: Pointer to data to print
 * @param words: The number of words to print to stdout.
 */
static inline void print_hex32(uint32_t *data, size_t words)
{
	int i;

	for (i = 0; i < words; ++i)
		printf("%08x", data[i]);
	printf("\n");
}


/**
 * Open a transfer descriptor on a provided driver.
 */
void transfer_init(void)
{
	struct termios toptions;
	char *driver = fval("socket");

	if (driver) {
		if (strstr(driver, "spi")) {
			// This is an SPI device, we have to use special Linux handling here.
			xfer.mode = TRANSFER_SPIDEV;
			xfer.fd = open(driver, O_RDWR);
			if (xfer.fd < 0) {
				fprintf(stderr, "Error: Failed to open driver %s\n", driver);
				return;
			}
		}else{
			// This is a UART device, we can treat it like a normal TTY.
			xfer.mode = TRANSFER_UART;
			xfer.fd = open(driver, O_RDWR | O_NOCTTY | O_NDELAY);
			if (xfer.fd < 0) {
				fprintf(stderr, "Error: Failed to open driver %s\n", driver);
				return;
			}

			tcgetattr(xfer.fd, &toptions);
			speed_t brate = B115200; 
			cfsetispeed(&toptions, brate);
			cfsetospeed(&toptions, brate);

			// 8N1
			toptions.c_cflag &= ~PARENB;
			toptions.c_cflag &= ~CSTOPB;
			toptions.c_cflag &= ~CSIZE;
			toptions.c_cflag |= CS8;
			toptions.c_cflag &= ~CRTSCTS; // No flow control
			toptions.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines
			toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
			// Make raw
			toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
			toptions.c_oflag &= ~OPOST;

			toptions.c_cc[VMIN] = 0;
			toptions.c_cc[VTIME] = 20;

			tcsetattr(xfer.fd, TCSANOW, &toptions);
		}
	}else{
		// No socket is provided, let's try FTDI.
		xfer.mode = TRANSFER_FTDI;
		xfer.mpsse = MPSSE(SPI0, 1000000, 0);
		if (!xfer.mpsse) {
			fprintf(stderr, "Error: Failed to open an MPSSE device!\n");
			return;
		}
	}
}

/**
 * Write a data packet to the GSC.
 *
 * @param data: Buffer containing data to transfer.
 * @param len: Size of data to TX.
 * 
 * @return Number of bytes TX'd.
 */
int transfer_write(void *data, size_t len)
{
	switch (xfer.mode) {
		case TRANSFER_FTDI:
			Start(xfer.mpsse);
			//Write(xfer.mpsse, data, len);
			if (!Transfer(xfer.mpsse, data, len)) {
				fprintf(stderr, "Error: Failed to transfer: %s\n", ErrorString(xfer.mpsse));
				return -1;
			}
			Stop(xfer.mpsse);
			return len;
			break;
		case TRANSFER_SPIDEV:
			// SPIDEV should function like a standard socket, Read and Write like normal.
		case TRANSFER_UART:
			// UART is just a normal TTY. Read and Write like normal.
			return write(xfer.fd, data, len);
			break;
		default:
			return -1;
			break;
	}

	return -1;
}



/**
 * Read a transferred response from the GSC.
 *
 * @param out: Buffer to place read data at.
 * @param len: Size of data to RX.
 * 
 * @return Number of bytes RX'd.
 */
int transfer_read(void *out, size_t len)
{
	switch (xfer.mode) {
		case TRANSFER_FTDI:
			// Transfer over EC's mpsse.c driver
			Start(xfer.mpsse);
			out = Read(xfer.mpsse, len);
			Stop(xfer.mpsse);
			return len;
			break;
		case TRANSFER_SPIDEV:
		case TRANSFER_UART:
			// UART is just a normal TTY. Read and Write like normal.
			return read(xfer.fd, out, len);
			break;
		default:
			return -1;
			break;
	}

	return -1;
}


/**
 * Synchronize the GSC rxbuf pointer with ours.
 *
 * @return 1 on success, 0 on failure.
 */
int transfer_syncronize()
{
	char c = 'e';

	for (int i = 0; i < 1024; i++){
		transfer_write("0", 1);

		fflush(NULL);
		usleep(25 * 1000); // Give the write time to finish

		// If we get something back, then we're synchronized.
		int readdata = transfer_read(&c, 1);
		if (readdata == 1)
			return 1;

		usleep(25 * 1000); // Give the read time to process
	}

	return 0;
}

/**
 * Close the transfer descriptor.
 */
void transfer_close(void)
{
	switch (xfer.mode) {
		case TRANSFER_FTDI:
			Close(xfer.mpsse);
			break;
		case TRANSFER_SPIDEV:
		case TRANSFER_UART:
			close(xfer.fd);
			break;
	}
}

/** 
 * Transfer a chunk of data over a provided channel to a connected target.
 * 
 * @param data:  Pointer to the chunk of data to be transferred.
 * @param size:  Size of the input data to be transferred.
 * @param adr:   Offset in the target's flash memory for the sent data
 *               to be stored.
 * @param frame_start: Starting frame number. Usually should be 0.
 * 
 * @return The number of written frames to the target.
 */
int transfer_chunk(const uint8_t *data, size_t size, 
                   uint32_t adr, int frame_start)
{
	uint8_t txbuf[1024];
	uint8_t rxbuf[SHA256_DIGEST_LENGTH];
	uint8_t digest[SHA256_DIGEST_LENGTH];
	struct bootstrap_pkt *pkt = (struct bootstrap_pkt *)(txbuf);
	int i, frame, retries;
	uint32_t offset = 0;

	// Start at the specified frame.
	frame = frame_start;

	// Initialize image transfer at the start of the GSC rxbuf.
	transfer_init();
	transfer_syncronize();

	while (true) {
		// If there's no data left to send, we're done.
		if (!size)
			break;

		// Prepare the bootstrap packet.
		memcpy(&pkt->data, data + offset, MIN(transfer_chunk_size, size));

		pkt->flash_offset = adr + offset;
		pkt->frame_num = frame;

		SHA256((uint8_t *) &pkt->frame_num, 
			sizeof(rxbuf) - SHA256_DIGEST_LENGTH, 
			(uint8_t *) &pkt->hash);

		
		// Send the packet and validate the response.
		if (fbool("verbose"))
			printf("Writing block at 0x%x\n", pkt->flash_offset);
		transfer_write(txbuf, sizeof(txbuf));
		transfer_read(rxbuf, sizeof(rxbuf));

		// Hash our full packet and remove NULL bytes from the result.
		SHA256(txbuf, sizeof(txbuf), digest);
		for (i = 0; i < sizeof(digest); ++i) {
			if (!digest[i])
				digest[i] |= 1;
		}

		if (memcmp(rxbuf, digest, SHA256_DIGEST_LENGTH)) {
			if (retries > 0) {
				fprintf(stderr, "Incorrect packet recieved, trying again...\n");
				if (fbool("verbose")) {
					printf("Debug: got");
					print_hex32(digest, SHA256_DIGEST_WORDS);
					printf("vs. expected ");
					print_hex32(rxbuf, SHA256_DIGEST_WORDS);
				}

				retries--;
				continue;
			}else{
				fprintf(stderr, "Error: Retry counter expired! Bailing out.\n");
				return -1;
			}
		}

		retries = MAX_RETRIES_COUNTER; // We give MAX_RETRIES_COUNTER retries for each packet.
		frame++;
		offset += transfer_chunk_size;
		size -= transfer_chunk_size;
	}

	transfer_close();

	return pkt->frame_num;
}


/**
 * End transfer on the provided driver.
 *
 * @return 0 on Success.
 */
int transfer_end(void)
{
	uint8_t txbuf[1024];
	struct bootstrap_pkt *pkt = (struct bootstrap_pkt *)txbuf;
	
	transfer_init();

	memset(txbuf, 0, sizeof(txbuf));
	pkt->frame_num = -1; 
	pkt->flash_offset = 0;
	SHA256(&pkt->frame_num, sizeof(txbuf) - SHA256_DIGEST_LENGTH, pkt->hash);

	transfer_write(txbuf, sizeof(txbuf));

	printf("Info: Transfer ended. If Rescue/Bootstrap did not end, perform a battery cutoff.\n");
	transfer_close();

	return 0;
}