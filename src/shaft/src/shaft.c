/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

/* Shaft - GSC RW Firmware Downgrader
 * Massive credits to the chromium authors for all of the documentation and
 * a lot of code pulled from src/platform/cr50 and src/platform/gsc-utils,
 * this wouldn't have been possible without them!
 *
 * This utility (written with love by Hannah <3) is designed to exploit the
 * lack of validity in the "cr50-rescue" feature  to downgrade the RW
 * sections of the Cr50 and Ti50 firmware. To compile it, simply install libcrypto and
 * libusb-1.0 (static copies can be found in the lib folder of the repo)
 * and run "make" in the root of the repo and hopefully everything should
 * compile smoothly!
 *
 * Huge thanks to Evelyn for helping write the UART serial code
 * and testing everything!
 */

#include <asm/byteorder.h>
#include <ctype.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "args.h"
#include "bootcon.h"
#include "chip_config.h"
#include "endorsement.h"
#include "header.h"
#include "rescue.h"
#include "signed_header.h"
#include "spiflash.h"
#include "transfer.h"
#include "version.h"

uint8_t *read_file(const char *path, size_t *size)
{
    FILE *fp;
	struct stat st;
	
    fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Failed to open file '%s'. %s\n", path, strerror(errno));
        exit(1);
    }
	
    if (fstat(fileno(fp), &st)) {
        fprintf(stderr, "Error: Failed to stat file '%s'. %s\n", path, strerror(errno));
        exit(1);
    }

    void *data = malloc(st.st_size);
    if (!data) {
        fprintf(stderr, "Error: Failed to allocate memory for provided file '%s'. %s\n", path, strerror(errno));
        exit(1);
    }

    if (fread(data, st.st_size, 1, fp) != 1) {
        fprintf(stderr, "Error: Failed to read file '%s'. %s\n", path, strerror(errno));
        exit(1);
    }
	
    fclose(fp);
	*size = st.st_size;
	
    return data;
}

int main(int argc, char **argv)
{
	parse_args(argc, argv);
	if (argc < 2) {
		fprintf(stderr, "Error: Expected more arguments.\n");
		show_info(1);
	}
	if (fbool("help")) show_info(0);
	if (fbool("version")) show_ver(0);

	if (fbool("output")) {
		size_t len;
		void *data = read_file(fval("firmware"), &len);
		uint32_t rw_offset, image_base, image_size = 0;
		struct SignedHeader *hdr;
		
		rw_offset = find_rw_header(data, len, CONFIG_RO_SIZE);
		hdr = (struct SignedHeader *)(data + rw_offset);
		image_base = hdr->ro_base;
		image_size = (hdr->magic == MAGIC_DAUNTLESS)
			? CONFIG_DT_RW_SIZE : CONFIG_RW_SIZE;

		if (hdr->magic == MAGIC_DAUNTLESS)
			image_base -= CONFIG_DT_PROGRAM_MEMORY_BASE;
		else
			image_base -= CONFIG_PROGRAM_MEMORY_BASE;

		// Rescue will not succeed if a valid header isn't placed in RW_A.
		if (!valid_signed_header(hdr)) {
			fprintf(stderr, "Error: Image RW_A is not valid! Rescue will not succeed!\n");
			return 1;
		}

		bin2rec(data, image_size, image_base, fval("output"));

		return 0;
	}

	if (getuid() != 0) {
		fprintf(stderr, "Error: Please run Shaft as root.\n");
		return 1;
	}

	if (fbool("bootcon")) {
		send_bootcon();
		return 0;
	}

	if (fbool("rescue")) {
		int frame = 0;
		char *bin = fval("firmware");
		char *tty = fval("socket");
		size_t len;
		void *data = read_file(bin, &len);
		uint32_t rw_offset, image_base;
		struct SignedHeader *hdr;

		rw_offset = find_rw_header(data, len, CONFIG_RO_SIZE);
		hdr = (struct SignedHeader *)(data + rw_offset);
		image_base = hdr->ro_base;

		if (hdr->magic == MAGIC_DAUNTLESS)
			image_base -= CONFIG_DT_PROGRAM_MEMORY_BASE;
		else
			image_base -= CONFIG_PROGRAM_MEMORY_BASE;

		// Rescue will not succeed if a valid header isn't placed in RW_A.
		if (!valid_signed_header(hdr)) {
			fprintf(stderr, "Error: Image RW_A is not valid! Rescue will not succeed!\n");
			return 1;
		}

		printf("Info: Engaging Rescue mode.\n");
		engage_rescue(tty);

		if (fbool("certificate"))
			populate_endorsement(data);
		
		printf("Info: Writing provided firmware over UART.\n");

		frame = transfer_chunk(hdr, hdr->image_size, image_base, 0);

		if (fbool("extortion")) 
			extort_rescue(tty, frame);
		transfer_end();

		free(data);
		return 0;
	}

	if (fbool("bootstrap")) {
		char *bin = fval("firmware");
		size_t len;
		void *data = read_file(bin, &len);
		uint32_t rw_offset, image_base;
		struct SignedHeader *hdr;

		rw_offset = find_rw_header(data, len, CONFIG_RO_SIZE);
		hdr = (struct SignedHeader *)(data + rw_offset);
		image_base = hdr->ro_base;

		if (hdr->magic == MAGIC_DAUNTLESS)
			image_base -= CONFIG_DT_PROGRAM_MEMORY_BASE;
		else
			image_base -= CONFIG_PROGRAM_MEMORY_BASE;

		if (!valid_signed_header(hdr)) {
			fprintf(stderr, "Error: Image RO_A is not valid! Bootstrap will not succeed!\n");
			return 1;
		}

		if (fbool("certificate"))
			populate_endorsement(data);

		printf("Info: Writing provided firmware over SPI.\n");
		transfer_chunk(data, len, image_base, 0);

		transfer_end();
		free(data);
		return 0;
	}

	// Should only be reached if no valid arguments were provided.
	fprintf(stderr, "Error: Expected a valid argument.\n");
	show_info(1);
}
