/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#include <asm/byteorder.h>
#include <endian.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "args.h"
#include "app_nugget.h"
#include "signed_header.h"
#include "board_id.h"
#include "chip_config.h"
#include "common.h"
#include "nuggets.h"
#include "smiko.h"
#include "tpm.h"
#include "tpm_vendor_cmds.h"
#include "usb.h"
#include "verify.h"

#include "vendor_cmds.h"

union gsc_start_resp start_resp;

uint32_t active_rw_offset;
uint32_t active_ro_offset;
uint32_t inactive_rw_offset;
uint32_t inactive_ro_offset;
enum gsc_device firmware;

int get_board_id(struct board_id *bid)
{
	size_t size = sizeof(*bid);

	if (fbool("verbose")) printf("Debug: Fetching board ID.\n");
	if (xfer_type == XFER_TPM_SOCKET) {
		if (tpm_send_payload(0, 0, bid, size, VENDOR_CC_GET_BOARD_ID) != 0) {
			fprintf(stderr, "Error: Failed to send board ID request to TPM.\n");
			return -1;
		}
		if (tpm_read_response(bid, &size) != VENDOR_RC_SUCCESS) {
			fprintf(stderr, "Error: Failed to read board ID from TPM.\n");
			return -1;
		}
	}else if (xfer_type == XFER_DT_SPI_DRIVER || xfer_type == XFER_CT_SPI_DRIVER) {
		nos_send_payload(0, 0, NULL, 0, NUGGET_PARAM_BOARD_ID, bid, &size);
	}else if (xfer_type == XFER_SUZYQ_USB) {
		usb_send_payload(0, 0, bid, size, VENDOR_CC_GET_BOARD_ID, bid, &size);
	}

	return 0;
}

int set_board_id(struct board_id *bid)
{
	size_t size;

	if (fbool("verbose")) printf("Debug: Setting board ID to %08x:%08x:%08x", 
							bid->id, bid->mask, bid->flags);

	if (xfer_type == XFER_TPM_SOCKET) {
		uint32_t command_body[2];
		size = sizeof(command_body);

		command_body[0] = htobe32(bid->id);
		command_body[1] = htobe32(bid->flags);

		if (tpm_send_payload(0, 0, command_body, size, VENDOR_CC_SET_BOARD_ID) != 0) {
			fprintf(stderr, "Error: Failed to send set board ID request to TPM.\n");
			return -1;
		}
		if (tpm_read_response(command_body, &size) != 0) {
			fprintf(stderr, "Error: Failed to read set board ID request to TPM.\n");
			return -1;
		}
	}else if (xfer_type == XFER_SUZYQ_USB) {
		uint32_t command_body[2];
		size = sizeof(command_body);

		command_body[0] = htobe32(bid->id);
		command_body[1] = htobe32(bid->flags);

		usb_send_payload(0, 0, command_body, size, VENDOR_CC_SET_BOARD_ID, command_body, &size);
	}

	return 0;
}

int get_cr50_info(struct sys_info_repsonse *info)
{
	size_t size = sizeof(*info);

	if (fbool("verbose")) printf("Debug: Getting Cr50 sysinfo.\n");
	if (xfer_type == XFER_TPM_SOCKET) {
		if (tpm_send_payload(0, 0, NULL, 0, VENDOR_CC_SYSINFO)) {
			fprintf(stderr, "Error: Failed to send dev ID request to TPM.\n");
			return -1;
		}
		if (tpm_read_response(info, &size) != VENDOR_RC_SUCCESS) {
			fprintf(stderr, "Error: Failed to read dev ID from TPM.\n");
			return -1;
		}
	}else if (xfer_type == XFER_SUZYQ_USB) {
		usb_send_payload(0, 0, NULL, 0, VENDOR_CC_SYSINFO, info, &size);
	}

	return 0;
}

int get_wp_state(uint8_t *ret)
{
	size_t size = sizeof(*ret);

	if (fbool("verbose")) printf("Debug: Getting Cr50 WP state.\n");
	if (xfer_type == XFER_TPM_SOCKET) {
		if (tpm_send_payload(0, 0, NULL, 0, VENDOR_CC_WP)) {
			fprintf(stderr, "Error: Failed to send WP request to TPM.\n");
			return -1;
		}
		
		if (tpm_read_response(ret, &size)) {
			fprintf(stderr, "Error: Failed to read WP state from TPM.\n");
			return -1;
		}
	}else if (xfer_type == XFER_SUZYQ_USB) {
		usb_send_payload(0, 0, ret, size, VENDOR_CC_WP, ret, &size);
	}

	return 0;
}

int get_boot_mode(uint8_t *ret)
{
	size_t size = sizeof(*ret);

	if (fbool("verbose")) printf("Debug: Getting EC Boot Mode.\n");
	if (xfer_type == XFER_TPM_SOCKET) {
		if (tpm_send_payload(0, 0, NULL, 0, VENDOR_CC_GET_BOOT_MODE)) {
			fprintf(stderr, "Error: Failed to send EC Boot request to TPM.\n");
			return -1;
		}
		
		if (tpm_read_response(ret, &size)) {
			fprintf(stderr, "Error: Failed to read EC boot state from TPM.\n");
			return -1;
		}
	}else if (xfer_type == XFER_SUZYQ_USB) {
		usb_send_payload(0, 0, ret, size, VENDOR_CC_GET_BOOT_MODE, ret, &size);
	}

	return 0;
}

int get_console_lock(uint8_t *ret)
{
	size_t size = sizeof(*ret);

	if (fbool("verbose")) printf("Debug: Getting CCD Console State...\n");
	if (xfer_type == XFER_TPM_SOCKET) {
		if (tpm_send_payload(0, 0, NULL, 0, VENDOR_CC_GET_LOCK)) {
			fprintf(stderr, "Error: Failed to send CCD request to TPM.\n");
			return -1;
		}
		if (tpm_read_response(ret, &size)) {
			fprintf(stderr, "Error: Failed to read CCD state from TPM.\n");
			return -1;
		}
	}else if (xfer_type == XFER_SUZYQ_USB) {
		usb_send_payload(0, 0, ret, size, VENDOR_CC_GET_LOCK, ret, &size);
	}

	return 0;
}

int get_cr50_metrics(struct cr50_stats_response *resp)
{
	size_t size = sizeof(*resp);

	if (fbool("verbose")) printf("Debug: Getting Cr50 Metrics...\n");
	if (xfer_type == XFER_TPM_SOCKET) {
		if (tpm_send_payload(0, 0, NULL, 0, VENDOR_CC_GET_CR50_METRICS) != 0) {
			fprintf(stderr, "Error: Failed to sent Cr50 Metrics request to TPM.\n");
			return -1;
		}
		if (tpm_read_response(resp, &size)) {
			fprintf(stderr, "Error: Failed to read Cr50 Metrics from TPM.\n");
			return -1;
		}
	}else if (xfer_type == XFER_SUZYQ_USB) {
		usb_send_payload(0, 0, resp, size, VENDOR_CC_GET_CR50_METRICS, resp, &size);
	}

	return 0;
}

enum gsc_device get_chip_id(void)
{
	uint32_t major = be32toh(start_resp.rpdu.shv[1].major);
	struct get_chip_id_response response;
	size_t response_size = sizeof(response);

	if (fbool("verbose")) printf("Debug: Checking GSC type.\n");
	
	if (major >= 30 && major < 40)
		return GSC_DEVICE_NT;
	else if (major >= 20 && major < 30)
		return GSC_DEVICE_DT;
	else if (major < 10)
		return GSC_DEVICE_H1;

	/* If the major version didn't fall between those ranges,
	 * let's check the chip ID. 
	 */
	if (xfer_type == XFER_TPM_SOCKET) {
		if (!tpm_send_payload(0, 0, NULL, 0, VENDOR_CC_GET_CHIP_ID)) {
			fprintf(stderr, "Error: Failed to send GET_CHIP_ID request.\n");
			return GSC_DEVICE_H1; // This will be our default.
		}

		if (!tpm_read_response(&response, &response_size)) {
			fprintf(stderr, "Error: Failed to read GET_CHIP_ID response.\n");
			return GSC_DEVICE_H1; // This will be our default.
		}
	}else if (xfer_type == XFER_SUZYQ_USB) {
		usb_send_payload(0, 0, NULL, 0, VENDOR_CC_GET_CHIP_ID, &response, &response_size);
	}

	switch (response.tpm_did_vid) {
		case 0x50666666:
			return GSC_DEVICE_NT;
		case 0x504a6666:
			return GSC_DEVICE_DT;
		case 0x00281ae0:
			return GSC_DEVICE_H1;
	}

	fprintf(stderr, "Error: Failed to identify GSC chip revision, defaulting to H1.\n");

	return GSC_DEVICE_H1; // This will be our default.
}

static unsigned int calculate_block_digest(const void *data, size_t data_size, 
                                    uint32_t offset, bool use_sha256)
{
	EVP_MD_CTX *ctx = EVP_MD_CTX_new();
	if (!use_sha256) 
		EVP_DigestInit_ex(ctx, EVP_sha1(), NULL);
	else
		EVP_DigestInit_ex(ctx, EVP_sha256(), NULL); // Ti50 uses sha256 for validity
	
	uint32_t block_base = htobe32(offset);
	EVP_DigestUpdate(ctx, &block_base, sizeof(block_base));
	EVP_DigestUpdate(ctx, data, data_size);
	
	uint8_t full_digest[SHA256_DIGEST_LENGTH]; // SHA256 length will always copy all of SHA1 as well
	unsigned int hashlen;
	uint32_t block_digest;
	EVP_DigestFinal(ctx, full_digest, &hashlen);
	memcpy(&block_digest, full_digest, MIN(sizeof(block_digest), hashlen));
	EVP_MD_CTX_free(ctx);

	return block_digest;
}

int transfer_chunk(uint8_t *data_ptr, uint32_t addr, size_t data_len)
{
	struct board_id bid;
	uint32_t payload_size, target_addr;
	struct SignedHeader *hdr = (struct SignedHeader *)data_ptr;


	if (fbool("verbose")) printf("Debug: Beginning chunk transfer.\n");


	while (data_len && (data_ptr[data_len - 1] == 0xff)) 
		data_len--; // Cut off tailing 0xFF bytes
	data_len = (data_len + 3) & ~3; // Make sure everything is aligned by 4-bytes for the flash controller

	target_addr = inactive_rw_offset;

	

	get_board_id(&bid);

	if (check_board_id_vs_header(&bid, hdr))
		fprintf(stderr, "Warning: Board ID will not match in firmware.\n");

	if (fbool("verbose")) {
		printf("Debug: Version in Header: %d.%d.%d\n", hdr->epoch_, hdr->major_, hdr->minor_);
		printf("Debug: Version in Flash: %d.%d.%d\n", be32toh(start_resp.rpdu.shv[1].epoch), be32toh(start_resp.rpdu.shv[1].major), be32toh(start_resp.rpdu.shv[1].minor));
		printf("Debug: Board ID in Flash: %08x:%08x:%08x\n", be32toh(bid.id), be32toh(bid.mask), be32toh(bid.flags));
		printf("Debug: Board ID in Header: %08x:%08x:%08x\n", be32toh(hdr->board_id.id) ^ SIGNED_HEADER_PADDING, be32toh(hdr->board_id.mask) ^ SIGNED_HEADER_PADDING, be32toh(hdr->board_id.flags) ^ SIGNED_HEADER_PADDING);
	}

	while(data_len) {
		payload_size = MIN(data_len, SIGNED_TRANSFER_SIZE);
		unsigned int digest = calculate_block_digest(data_ptr, payload_size, target_addr, firmware != GSC_DEVICE_H1);
		uint8_t error_code[4];
		size_t rxed_size = sizeof(error_code);

		if (fbool("verbose")) printf("Debug: Writing block to destined base 0x%x\n", target_addr);

		if (xfer_type == XFER_TPM_SOCKET) {
			if (tpm_send_payload(digest, target_addr, data_ptr, payload_size, EXTENSION_FW_UPGRADE) != 0) {
				fprintf(stderr, "Failed to write provided stock firmware image.\n");
				return -1;
			}

			tpm_read_response(error_code, &rxed_size);

			if (rxed_size != 1) {
				fprintf(stderr, "Error: Unexpected return size from upgrade %zd\n", rxed_size);
				return -1;
			}

			switch (error_code[0]) {
				case UPGRADE_ERASE_FAILURE:
					fprintf(stderr, "Error: Failed to erase target block for writing.\n");
					break;
				case UPGRADE_DATA_ERROR:
					fprintf(stderr, "Error: Incorrect hash on sent data.\n");
					break;
				case UPGRADE_VERIFY_ERROR:
					fprintf(stderr, "Error: Failed to verify the provided firmware was written, but assuming success.\n");
					return 0;
					break;
				case UPGRADE_ROLLBACK_ERROR:
					fprintf(stderr, "Error: Provided image is older than the current firmware. Use Shaft if you need to downgrade.\n");
					break;
				case UPGRADE_RATE_LIMIT_ERROR:
					fprintf(stderr, "Error: Firmware upgrade requested too soon; please wait 60 seconds before trying again.\n");
					break;
				case UPGRADE_TRUNCATED_HEADER_ERROR:
					fprintf(stderr, "Error: Sent header truncated, should be 1024 bytes.\n");
					break;
				case UPGRADE_BOARD_ID_ERROR:
					fprintf(stderr, "Error: Mismatched board ID in image. Use Shaft if you need to bypass the Board ID check.\n");
					break;
				case UPGRADE_BOARD_FLAGS_ERROR:
					fprintf(stderr, "Error: Mismatched board flags in image. Use Shaft if you need to bypass the Board ID check.\n");
					break;
				case UPGRADE_DEV_ID_MISMATCH_ERROR:
					fprintf(stderr, "Error: Mismatched Dev ID in image.\n");
					break;
			}
			
			if (error_code[0])
				return -1;
		}else if (xfer_type == XFER_CT_SPI_DRIVER || xfer_type == XFER_DT_SPI_DRIVER) {
			nos_send_payload(digest, target_addr, data_ptr, payload_size, NUGGET_PARAM_FLASH_BLOCK, error_code, &rxed_size);
		}else{
			uint8_t *transfer_data_ptr = data_ptr;
			struct update_pdu updu;
			updu.block_size = htobe32(payload_size + sizeof(struct update_pdu));;
			updu.cmd.block_digest = digest;
			updu.cmd.block_base = target_addr;
			
			do_xfer(&uep, &updu, sizeof(struct update_pdu), NULL, 0, 0, NULL);

			for (int transfer_size = 0; transfer_size < payload_size;) {
				int chunk_size = MIN(uep.chunk_len, payload_size - transfer_size);
				do_xfer(&uep, transfer_data_ptr, chunk_size, NULL, 0, 0, NULL);
				transfer_data_ptr += chunk_size;
				transfer_size += chunk_size;
			}
		}

		target_addr += payload_size; // Target the area directly after what we just wrote
		data_ptr += payload_size; // Start copying from the next chunk
		data_len -= payload_size; // Subtract the amount we just wrote
	}

	return 0;
}

static bool valid_header_ranges(const struct SignedHeader *hdr)
{
	uint32_t ro_adr, rw_adr;
	bool rw = false;
	char *section;

	if (firmware != GSC_DEVICE_H1) {
		/* Dauntless's RW's can move around a bit, so we treat it specially. */
		ro_adr = inactive_ro_offset + CONFIG_DT_PROGRAM_MEMORY_BASE;
		rw_adr = inactive_rw_offset + CONFIG_DT_PROGRAM_MEMORY_BASE;

		/* If the addresses match the inactive banks, we don't need
		 * to do any boundary checking.
		 */
		if (hdr->ro_base == ro_adr)
			goto section_requested;

		/* The addresses don't align perfectly, the RW will have to move.
		 * Let's make sure this move makes sense before writing.
		 */
		/* Headers must be 2kB aligned */
		if (hdr->ro_base % 0x800 != 0)
			return false;
		
		/* RO images will always pass the inactive bank checks above,
		 * so we only check the RW boundaries here.
		 */
		rw = true;
		if ((hdr->ro_base > CONFIG_DT_PROGRAM_MEMORY_BASE + CONFIG_RO_SIZE &&
			hdr->ro_base < CONFIG_DT_PROGRAM_MEMORY_BASE + CFG_DT_FLASH_HALF) ||
			(hdr->ro_base > CONFIG_DT_PROGRAM_MEMORY_BASE + CFG_DT_FLASH_HALF + CONFIG_RO_SIZE &&
			hdr->ro_base < CONFIG_DT_PROGRAM_MEMORY_BASE + CFG_DT_FLASH_HALF * 2))
			goto section_requested;
		else
			return false;
	}else{
		/* Haven's RWs never move, so we can hardcode ro_base checks. */
		ro_adr = inactive_ro_offset + CONFIG_PROGRAM_MEMORY_BASE;
		rw_adr = inactive_rw_offset + CONFIG_PROGRAM_MEMORY_BASE;

		if (hdr->ro_base != ro_adr && hdr->ro_base != rw_adr)
			return false;
		
		if (hdr->ro_base == (CONFIG_PROGRAM_MEMORY_BASE + CONFIG_RO_SIZE) ||
			hdr->ro_base == (CONFIG_PROGRAM_MEMORY_BASE + CFG_FLASH_HALF + CONFIG_RO_SIZE))
			rw = true;
	}

	/* Next, let's only return true if the section we're targetting
	 * falls in the ranges of the region specified by the command line.
	 */
	section_requested:
	/* If no section is specified, only allow writes to the RW. */
	section = (fbool("section")) ? fval("section") : "RW";

	/* Allow all writes. */
	if (!strcmp(section, "BOTH"))
		return true;
	
	/* Only allow writes if we're targetting the correct section. */
	if (!strcmp(section, "RO") && rw == false)
		return true;
	if (!strcmp(section, "RW") && rw == true)
		return true;

	/* If an invalid --section argument was provided, do not write anything. */
	return false;
}

int write_firmware(const char *path)
{
	uint32_t size, offset = 0;
	int rc, writes = 0;
	void *data;
	struct SignedHeader *hdr;

	data = read_file(path, &size);

	do {
		hdr = (struct SignedHeader *)(data + offset);

		if (!valid_signed_header(hdr))
			goto proceed;
		
		/* Only write to the inactive banks. */
		if (!valid_header_ranges(hdr)) 
			goto proceed;
		
		rc = transfer_chunk(hdr, hdr->ro_base, hdr->image_size);
		if (rc)
			return rc;

		writes++;

		proceed:
		offset = round_up_2kb(offset + 1);
	} while (offset < (size / 2));

	if (!writes)
		fprintf(stderr, "Error: Failed to find any valid images in provided firmware!\n");
	else
		printf("Info: Successfully wrote %d image sections\n", writes);

	return rc;
}


static int immediate_reset(void)
{
	if (!fbool("suzyq")) {
		if (tpm_send_payload(0, 0, NULL, 0, VENDOR_CC_IMMEDIATE_RESET) != 0) {
			fprintf(stderr, "Error: Failed to finalize and send update request.\n");
			return -1;
		}
	}else{
		usb_send_payload(0, 0, NULL, 0, VENDOR_CC_IMMEDIATE_RESET, NULL, 0);
	}

	return 0;
}

// This will cause a GSC reset if a header was modified in the Flash.
static int finalize_changes(int post)
{
	uint8_t command_body[2] = {0, 100}; // 1 second delay
	size_t command_size = (post) ? 0 : sizeof(command_body);
	if (!fbool("suzyq")) {
		if (tpm_send_payload(0, 0, command_body, sizeof(command_body), (post) ? EXTENSION_POST_RESET : VENDOR_CC_TURN_UPDATE_ON) != 0) {
			fprintf(stderr, "Error: Failed to finalize and send update request.\n");
			return -1;
		}
	}else{
		usb_send_payload(0, 0, command_body, command_size, (post) ? EXTENSION_POST_RESET : VENDOR_CC_TURN_UPDATE_ON, NULL, 0);
	}

	return 0;
}


int reset_gsc(void)
{
	if (!strcmp(fval("reset"), "immediate"))
		return immediate_reset();
	if (!strcmp(fval("reset"), "post"))
		return finalize_changes(1);
	if (!strcmp(fval("reset"), "update"))
		return finalize_changes(0);

	return -1;
}

int connect_to_target(void)
{
	size_t rxed_size;

	if (!fbool("suzyq")) {
		if (!fbool("trunks")) {
			gsc_socket = open_socket();
			if (gsc_socket < 0) {
				fprintf(stderr, "Error: Failed to open a GSC socket.\n");
				return 1;
			}
		}

		if (tpm_send_payload(0, 0, NULL, 0, EXTENSION_FW_UPGRADE) != 0) {
			fprintf(stderr, "Error: Failed to write over TPM connection, cannot proceed.\n");
			return 1;
		}

		rxed_size = sizeof(start_resp);
		if (tpm_read_response(&start_resp, &rxed_size) != VENDOR_RC_SUCCESS) {
			fprintf(stderr, "Error: Failed to read response from TPM, cannot proceed.\n");
			return 1;
		}
	}else{
		// Extra variables only used to prevent 80+ character lines, thanks chromium devs
		const uint16_t subclass = USB_SUBCLASS_GOOGLE_CR50;
		const uint16_t protocol = USB_PROTOCOL_GOOGLE_CR50_NON_HC_FW_UPDATE;
		const uint16_t vid = G_VID;
		uint16_t pid;
		// TODO (Hannah): Try all 3 before quitting.
		if (fbool("dauntless"))
			pid = D2_PID;
		else if (fbool("nuvotitan"))
			pid = NT_PID;
		else
			pid = H1_PID;

		if (fbool("verbose")) printf("Debug: Searching for usb interface %08x:%08x\n", vid, pid); 
		if (usb_findit(NULL, vid, pid, subclass, protocol, &uep)) {
			// If we made it here, we didn't find any USB devices matching our needs.
			fprintf(stderr, "Error: Failed to find GSC interface, cannout proceed.\n");
			fprintf(stderr, "Tip: Try --dauntless or --nuvotitan?\n");
			return 1;
		}

		struct update_pdu updu;
		memset(&updu, 0, sizeof(updu));
		updu.block_size = htobe32(sizeof(updu));
		
		if (fbool("verbose")) printf("Debug: Sending USB xfer connection establishment request.\n");
		do_xfer(&uep, &updu, sizeof(updu), &start_resp, sizeof(start_resp), 1, &rxed_size);
	}

	if (rxed_size < 8) {
		fprintf(stderr, "Error: Unexpected response size from setup packet %zd\n", rxed_size);
		return 1;
	}

	uint32_t error_code = be32toh(start_resp.rpdu.return_value);
	if (error_code != VENDOR_RC_SUCCESS) {
		fprintf(stderr, "Error: Target returned error code %d.\n", error_code);
		return 1;
	}

	firmware = get_chip_id();

	inactive_rw_offset = be32toh(start_resp.rpdu.backup_rw_offset);
	inactive_ro_offset = be32toh(start_resp.rpdu.backup_ro_offset);
	
	switch (firmware) {
		default:
		case GSC_DEVICE_H1:
			active_ro_offset = (inactive_ro_offset == 0x40000) ? 0x0 : 0x40000;
			active_rw_offset = (inactive_rw_offset == 0x44000) ? 0x4000 : 0x44000;
			break;
		case GSC_DEVICE_DT:
		case GSC_DEVICE_NT:
			active_ro_offset = (inactive_ro_offset == 0x80000) ? 0x0 : 0x80000;
			active_rw_offset = (inactive_rw_offset > 0x80000) ? inactive_rw_offset - 0x80000 : inactive_rw_offset + 0x80000;
			break;
	}

	if (fbool("verbose")) printf("Debug: Connection established to target successfully.\n");

	return 0;
}