/* Copyright 2017 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <asm/byteorder.h>
#include <ctype.h>
#include <endian.h>
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "args.h"
#include "chip_config.h"
#include "common.h"
#include "trunks.h"
#include "tpm.h"
#include "tpm_vendor_cmds.h"

int gsc_socket;

struct tpm_pkt {
	__be16 tag;
	__be32 length;
	__be32 ordinal;
	__be16 subcmd;
	union {
		struct {
			__be32 digest;
			__be32 address;
			char data[0];
		} upgrade;
		struct {
			char data[0];
		} command;
	};
} __attribute__((packed));

int tpm_send_payload(unsigned int digest, unsigned int addr, 
                 const void *data, int size, uint16_t subcmd)
{
	static uint8_t outbuf[MAX_TX_BUF_SIZE];
	struct tpm_pkt *out = (struct tpm_pkt *)outbuf;
	int len, done;
	void *payload;
	size_t header_size;
	
	out->tag = htobe16(0x8001);
	out->subcmd = htobe16(subcmd);

	out->ordinal = htobe32((subcmd <= LAST_EXTENSION_COMMAND) ? 
				CONFIG_EXTENSION_COMMAND : TPM_CC_VENDOR_BIT_MASK);
	
	if (subcmd == EXTENSION_FW_UPGRADE) {
		out->upgrade.digest = digest;
		out->upgrade.address = htobe32(addr);
		header_size = offsetof(struct tpm_pkt, upgrade.data);
	}else{
		header_size = offsetof(struct tpm_pkt, command.data);
	}

	payload = outbuf + header_size;
	len = size + header_size;

	out->length = htobe32(len);
	memcpy(payload, data, size);

	if (!fbool("trunks")) 
		done = write(gsc_socket, out, len);
	else
		done = trunks_write(out, len);

	if (done < 0) {
		fprintf(stderr, "Error: Failed to write to TPM, %s.\n", strerror(errno));
		return 1;
	}else if (done != len) {
		fprintf(stderr, "Error: Expected to write %d bytes to TPM, instead wrote %d. %s\n", len, done, strerror(errno));
		return 1;
	}

	if (fbool("verbose")) printf("Debug: Wrote %d bytes to TPM successfully.\n", done);

	return 0;
}

int tpm_read_response(void *response, size_t *response_size)
{
	static uint8_t raw_response[MAX_RX_BUF_SIZE + sizeof(struct tpm_pkt)];
	int response_offset = offsetof(struct tpm_pkt, command.data);
	const size_t rx_size = sizeof(raw_response);
	uint32_t rv;

	int read_count, len;

	if (!fbool("trunks")) {
		len = 0;
		do {
			uint8_t *rx_buf = raw_response + len;
			size_t rx_to_go = rx_size - len;

			read_count = read(gsc_socket, rx_buf, rx_to_go);

			len += read_count;
		} while (read_count);
	}else{
		len = trunks_read(raw_response, rx_size);
	}

	len = len - response_offset;
	if (len < 0) {
		fprintf(stderr, "Error: Problems reading from TPM, got %d bytes.\n", len + response_offset);
		return 1;
	}

	len = MIN(len, *response_size);
	memcpy(response, raw_response + response_offset, len);
	*response_size = len;

	memcpy(&rv, &((struct tpm_pkt *)raw_response)->ordinal, sizeof(rv));
	rv = be32toh(rv);

	if ((rv & VENDOR_RC_ERR) == VENDOR_RC_ERR) 
		rv &= ~VENDOR_RC_ERR;

	if (fbool("verbose")) printf("Debug: Read %d bytes from TPM.\n", len);

	return rv;
}