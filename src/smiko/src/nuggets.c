/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#include <stdint.h>
#include <stdio.h>


#include "app_nugget.h"
#include "smiko.h"
#include "tpm.h"


struct nos_pkt {
	uint16_t length;
	uint16_t version;
	uint16_t crc;
	uint16_t reply_len_hint;
} __attribute__((packed));

/* Send and recieve data over the Citadel/Dauntless driver. */
int nos_send_payload(unsigned int digest, unsigned int addr, 
					 const void *data, int size, uint16_t subcmd,
					 void *resp, size_t *resp_size)
{
	uint8_t txbuf[2048];
	struct nos_pkt *pkt = (struct nos_pkt *)txbuf;
	uint32_t rv = 0;

	pkt->length = sizeof(struct nos_pkt);
	
	return rv;
}

