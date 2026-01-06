/* Copyright 2017 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <asm/byteorder.h>
#include <ctype.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "trunks.h"


/* Helpers to convert between binary and hex ascii and back. */
static char to_hexascii(uint8_t c)
{
	if (c <= 9)
		return '0' + c;
	return 'a' + c - 10;
}

static int from_hexascii(char c)
{
	/* convert to lower case. */
	c = tolower(c);

	if (c < '0' || c > 'f' || ((c > '9') && (c < 'a')))
		return -1; /* Not an ascii character. */

	if (c <= '9')
		return c - '0';

	return c - 'a' + 10;
}

/* File handle to share between write and read sides. */
static FILE *tpm_output;
int trunks_write(const void *out, size_t len)
{
	const char *cmd_head = "PATH=\"${PATH}:/usr/sbin:/vendor/bin/hw\" ${TRUNKS_SEND_BIN:-trunks_send} --raw ";
	size_t head_size = strlen(cmd_head);
	char full_command[head_size + 2 * len + 1];
	size_t i;

	strcpy(full_command, cmd_head);
	/*
	 * Need to convert binary input into hex ascii output to pass to the
	 * trunks_send command.
	 */
	for (i = 0; i < len; i++) {
		uint8_t c = ((const uint8_t *)out)[i];

		full_command[head_size + 2 * i] = to_hexascii(c >> 4);
		full_command[head_size + 2 * i + 1] = to_hexascii(c & 0xf);
	}

	/* Make it a proper zero terminated string. */
	full_command[sizeof(full_command) - 1] = 0;
	tpm_output = popen(full_command, "r");
	if (tpm_output) return len;

	fprintf(stderr, "Error: failed to launch trunks_send --raw\n");
	return -1;
}

int trunks_read(void *buf, size_t max_rx_size)
{
	int i, pclose_rv, rv;
	/* +1 to account for '\n' added by trunks_send. */
	char response[max_rx_size * 2 + 1];

	if (!tpm_output) {
		fprintf(stderr, "Error: Attempt to read empty output from trunks_send\n");
		return -1;
	}

	rv = fread(response, 1, sizeof(response), tpm_output);
	if (rv > 0) rv -= 1; /* Discard the \n character added by trunks_send. */

	pclose_rv = pclose(tpm_output);
	if (pclose_rv < 0) {
		fprintf(stderr, "Error: pclose failed: error %d (%s)\n", errno, strerror(errno));
		return -1;
	}

	tpm_output = NULL;
	if (rv & 1) {
		fprintf(stderr, "Error: trunks_send returned odd number of bytes: %s\n", response);
		return -1;
	}

	for (i = 0; i < rv / 2; i++) {
		uint8_t byte;
		char c;
		int nibble;

		c = response[2 * i];
		nibble = from_hexascii(c);
		if (nibble < 0) {
			fprintf(stderr, "Error: trunks_send returned non hex character %c\n", c);
			return -1;
		}
		byte = nibble << 4;

		c = response[2 * i + 1];
		nibble = from_hexascii(c);
		if (nibble < 0) {
			fprintf(stderr, "Error: trunks_send returned non hex character %c\n", c);
			return -1;
		}
		byte |= nibble;

		((uint8_t *)buf)[i] = byte;
	}

	return rv / 2;
}
