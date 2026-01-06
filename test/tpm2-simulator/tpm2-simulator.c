// Copyright 2015 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modified by Hannah for usage without ChromeOS libraries (and for general
// usage improvements)

#include <asm/byteorder.h>
#include <fcntl.h>
#include <errno.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "arg_checks.h"
#include "vendor-code-string.h"
#include "tpm_vendor_cmds.h"

#include "_TPM_Init_fp.h"
#include "ExecCommand_fp.h"
#include "GetCommandCodeString_fp.h"
#include "Platform.h"
#include "Startup_fp.h"
#include "tpm_generated.h"

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

int gargc;
char **gargv;

int init_tpm(void)
{
	_plat__Signal_PowerOn();
	_TPM_Init();
	_plat__SetNvAvail();
	return 0;
}

int tpm; // Define the stream as a global variables
int create_pipes(char *socket)
{
	mkfifo(socket, O_CREAT | O_RDWR);

	tpm = open(socket, O_RDWR);
	if (tpm < 0) {
		fprintf(stderr, "TPM simulator: Error opening stream '%s': %s\n", socket, strerror(errno));
		return -1;
	}

	return 0;
}

int verify_rc(int rc, int expect)
{
	if (rc != expect) {
		fprintf(stderr, "Error: Bad RC %d, Expected RC %d\n", rc, expect);
		return -1;
	}

	return 0;
}

void show_info(int esc)
{
	printf("Usage: sudo %s <optional args>\n"
		"\n"
		"Arguments:\n"
		"-h, --help: Show this help and exit\n"
		"-s, --socket [/dev/tpm#]: Specify the output TPM socket (Default: /dev/swtpm0).\n"
		"-c, --cleanup: Cleanup the used sockets and exit\n"
		"-C, --clear: Clear the TPM\n"
		"-f, --force: Force the simulator to run even if not being run as root\n"
		"-v, --debug: Show extra debug logging on stdout\n"
		"\n", gargv[0]);
	exit(esc);
}

int main(int argc, char **argv) 
{
	gargc = argc;
	gargv = argv;
	if (fbool("--help","-h")) show_info(0);

	if (getuid() != 0 && !fbool("--force","-f")) {
		fprintf(stderr, "Error: tpm2-simulator must be run as root.\n");
		return -1;
	}

	char *socket = (fbool("--socket","-s")) ? fval("--socket","-s", 1) : "/dev/swtpm0";

	if (fbool("--cleanup","-c")) {
		printf("Info: Cleaning up.\n");
		if (remove(socket)) {
			fprintf(stderr, "Error: Failed to remove socket: %s\n", strerror(errno));
			return -1;
		}

		return 0;
	}

	if (fbool("--clear","-C")) {
		printf("Info: Clearing TPM NV data.\n");
		if (!remove("./NVChip")) {
			fprintf(stderr, "Error: Failed to erase NVChip file: %s\n", strerror(errno));
			return -1;
		}

		return 0;
	}

	if (create_pipes(socket)) {
		fprintf(stderr, "Error: Failed to create needed pipes.\n");
		return -1;
	}

	// Initialize TPM.
	if (init_tpm()) {
		fprintf(stderr, "Error: Failed to initialize TPM2.\n");
		return -1;
	}

	printf("TPM simulator: Initialization complete. Awaiting TPM2_Startup.\n");

	while (true) {
		unsigned char request[MAX_COMMAND_SIZE];
		unsigned int request_size;

		// Read request header.
		int done = read(tpm, request, 10);
		if (done < 0) {
			fprintf(stderr, "TPM simulator: Error receiving request header: %s\n", strerror(errno));
			return -1;
		}

		uint8_t *header = request;
		struct tpm_pkt *hdr = (struct tpm_pkt *)(header);
		INT32 header_size = 10;
		TPMI_ST_COMMAND_TAG tag;
		UINT32 command_size;
		TPM_CC command_code;
		UINT32 ordinal;

		TPM_RC rc = TPM_RC_SUCCESS;

		// Unmarshal request header to get request size and command code.
		rc = TPMI_ST_COMMAND_TAG_Unmarshal(&tag, &header, &header_size);
		verify_rc(rc, TPM_RC_SUCCESS);
		rc = UINT32_Unmarshal(&command_size, &header, &header_size);
		verify_rc(rc, TPM_RC_SUCCESS);
		rc = TPM_CC_Unmarshal(&command_code, &header, &header_size);
		verify_rc(rc, TPM_RC_SUCCESS);
		ordinal = be32toh(hdr->ordinal);

		// Read request body.
		if (request_size > 10) {
			done = read(tpm, request + 10, request_size - 10);
			if (done != (request_size - 10)) {
				fprintf(stderr, "TPM simulator: Error receiving request body: %s\n", strerror(errno));
				return -1;
			}
		}

		unsigned int response_size;
		uint8_t *response;

		if (ordinal == CONFIG_EXTENSION_COMMAND || ordinal == TPM_CC_VENDOR_BIT_MASK) {
			// Execute vendor/extension command
			printf("GSC simulator: Executing %s\n", GetVendorCodeString(command_code));
		}else{
			// Execute TPM command.
			printf("TPM simulator: Executing %s\n", GetCommandCodeString(command_code));
			ExecuteCommand(request_size, request, &response_size, &response);
		}
    
		// Write response.
		done = write(tpm, response, response_size);
		if (done < 0) {
			fprintf(stderr, "TPM simulator: Error writing response: %s", strerror(errno));
			return -1;
		}
	}

	fprintf(stderr, "Error: Main TPM2 loop broken, not sure what to do...");
	return 1;
}
