/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#include <asm/byteorder.h>
#include <endian.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "args.h"
#include "board_id.h"
#include "chip_config.h"
#include "common.h"
#include "scratch_reg1.h"
#include "smiko.h"
#include "signed_header.h"
#include "tpm.h"
#include "tpm_vendor_cmds.h"
#include "verify.h"

#include "vendor_cmds.h"

extern uint32_t active_rw_offset;
extern uint32_t active_ro_offset;
extern uint32_t inactive_rw_offset;
extern uint32_t inactive_ro_offset;
extern enum gsc_device firmware;

static char *base_to_image(uint32_t base)
{
	switch (firmware) {
		case GSC_DEVICE_H1:
		case GSC_DEVICE_CT:
			switch (base) {
				case 0x40000:
					return "RO_A";
				case 0x44000:
					return "RW_A";
				case 0x80000:
					return "RO_B";
				case 0x84000:
					return "RW_B";
				default:
					return "??";
			}
			break;
		case GSC_DEVICE_DT:
			if (base == 0x80000)
				return "RO_A";
			if (base >= 0x84000 && base < 0x100000)
				return "RW_A";
			if (base == 0x100000)
				return "RO_B";
			if (base >= 0x104000 && base < 0x184000)
				return "RW_B";
			return "??";
			break;
		default:
			return "??";
			break;
	}
}

int get_sysinfo(struct first_response_pdu rpdu)
{
	struct board_id bid;
	struct sys_info_repsonse info; 
	struct cr50_stats_response stats;
	uint8_t wp_state, boot_mode, console_locked;
	bool is_ti50, fwmp_en = false;
	uint32_t flash_offset;
	char *gsc_type;

	char *icon0, *icon1, *icon2, *icon3, *icon4, *icon5, *icon6, *icon7;
	
	is_ti50 = (firmware == GSC_DEVICE_DT || firmware == GSC_DEVICE_NT);

	get_board_id(&bid);
	get_cr50_info(&info);
	get_wp_state(&wp_state);
	get_boot_mode(&boot_mode);
	get_console_lock(&console_locked);
	if (!is_ti50) get_cr50_metrics(&stats);
	fwmp_en = (wp_state & WPV_FWMP_FORCE_WP_EN);

	switch (firmware) {
		case GSC_DEVICE_H1:
			flash_offset = CONFIG_PROGRAM_MEMORY_BASE;
			gsc_type = "Haven/H1, Cr50 Firmware";
			icon0 = "╔════════════════════╗--";
			icon1 = "║    ██╗  ██╗ ██╗    ║--";
			icon2 = "║    ██║  ██║███║    ║--";
			icon3 = "║    ███████║╚██║    ║--";
			icon4 = "║    ██╔══██║ ██║    ║--";
			icon5 = "║    ██║  ██║ ██║    ║--";
			icon6 = "║    ╚═╝  ╚═╝ ╚═╝    ║--";
			icon7 = "╚════════════════════╝--";
			break;
		case GSC_DEVICE_DT:
			flash_offset = CONFIG_DT_PROGRAM_MEMORY_BASE;
			gsc_type = "Dauntless/D2, Ti50/Acropora Firmware";
			icon0 = "╔════════════════════╗--";
			icon1 = "║  ██████╗ ██████╗   ║--";
			icon2 = "║  ██╔══██╗╚════██╗  ║--";
			icon3 = "║  ██║  ██║ █████╔╝  ║--";
			icon4 = "║  ██║  ██║██╔═══╝   ║--";
			icon5 = "║  ██████╔╝███████╗  ║--";
			icon6 = "║  ╚═════╝ ╚══════╝  ║--";
			icon7 = "╚════════════════════╝--";
			break;
		case GSC_DEVICE_NT:
			flash_offset = CONFIG_DT_PROGRAM_MEMORY_BASE;
			gsc_type = "OpenTitan/NT, Ti50 Firmware";
			icon0 = "╔════════════════════╗--";
			icon1 = "║ ███╗   ██╗████████╗║--";
			icon2 = "║ ████╗  ██║╚══██╔══╝║--";
			icon3 = "║ ██╔██╗ ██║   ██║   ║--";
			icon4 = "║ ██║╚██╗██║   ██║   ║--";
			icon5 = "║ ██║ ╚████║   ██║   ║--";
			icon6 = "║ ╚═╝  ╚═══╝   ╚═╝   ║--";
			icon7 = "╚════════════════════╝--";
			break;
		default:
			flash_offset = CONFIG_PROGRAM_MEMORY_BASE;
			gsc_type = "Unrecognized GSC Firmware";
			icon0 = "                        ";
			icon1 = "                        ";
			icon2 = "                        ";
			icon3 = "      Unknown Chip      ";
			icon4 = "                        ";
			icon5 = "                        ";
			icon6 = "                        ";
			icon7 = "                        ";
			break;
	}

	printf("                               %s\n", gsc_type);
	printf("                               ---------------------------------------------------------\n");
	printf("   %s    \e[1;31mInjection Method\e[0m: %s\n", icon0, (fbool("suzyq")) ? "SuzyQable/GSC Debug Board injection" : (fbool("trunks")) ? "ChromeOS trunks daemon software injection" : "GSC driver software injection");
	printf("   %s    \e[1;31mProtocol Version\e[0m: %d\n", icon1, be32toh(rpdu.protocol_version));
	printf("   %s    \e[1;31mBoard ID\e[0m: %08x:%08x:%08x\n", icon2, be32toh(bid.id), be32toh(bid.mask), be32toh(bid.flags));
	printf("   %s    \e[1;31mDev IDs\e[0m: 0x%08x, 0x%08x\n", icon3, be32toh(info.dev_id0), be32toh(info.dev_id1));
	printf("   %s    \e[1;31mActive RO Offset\e[0m: 0x%x (Physical Offset: 0x%x) (%s)\n", icon4, active_ro_offset, active_ro_offset + flash_offset, base_to_image(active_ro_offset + flash_offset));
	printf("   %s    \e[1;31mActive RW Offset\e[0m: 0x%x (Physical Offset: 0x%x) (%s)\n", icon5, active_rw_offset, active_rw_offset + flash_offset, base_to_image(active_rw_offset + flash_offset));
	printf("   %s    \e[1;31mBackup RO Offset\e[0m: 0x%x (Physical Offset: 0x%x) (%s)\n", icon6, inactive_ro_offset, inactive_ro_offset + flash_offset, base_to_image(inactive_ro_offset + flash_offset));
	printf("   %s    \e[1;31mBackup RW Offset\e[0m: 0x%x (Physical Offset: 0x%x) (%s)\n", icon7, inactive_rw_offset, inactive_rw_offset + flash_offset, base_to_image(inactive_rw_offset + flash_offset));
	printf("                               \e[1;31mRO FW Version\e[0m: %d.%d.%d\n", be32toh(rpdu.shv[0].epoch), be32toh(rpdu.shv[0].major), be32toh(rpdu.shv[0].minor));
	printf("                               \e[1;31mRW FW Version\e[0m: %d.%d.%d\n", be32toh(rpdu.shv[1].epoch), be32toh(rpdu.shv[1].major), be32toh(rpdu.shv[1].minor));
	printf("                               \e[1;31mKey IDs\e[0m: RO 0x%08x, RW 0x%08x\n", be32toh(rpdu.keyid[0]), be32toh(rpdu.keyid[1]));
	printf("                               \e[1;31mHardware Write Protection\e[0m: %s%s%s\n",
											(wp_state & WPV_FWMP_FORCE_WP_EN) ? "FWMP " : "", 
											(wp_state & WPV_FORCE) ? "Force " : "Battery ",
											(wp_state & WPV_ENABLE) ? "Enabled" : "Disabled");
	printf("                               \e[1;31mFirmware Management Parameters\e[0m: %s\n", (fwmp_en) ? "Developer Mode and CCD Locked" : "Cleared");
	printf("                               \e[1;31mEC Boot Mode\e[0m: %s\n", (!boot_mode) ? "Booted (Normal)" : "Not Booted");
	printf("                               \e[1;31mCCD Console State\e[0m: %s\n", (console_locked) ? "Locked" : "Unlocked");
	if (!is_ti50) printf("                               \e[1;31mBoard Properties\e[0m: %s%s%s\n", 
		(be32toh(stats.brdprop) & BOARD_PERIPH_CONFIG_SPI) ? "SPI TPM" : "I2C TPM",
		(be32toh(stats.brdprop) & BOARD_NO_RO_UART) ? ", Rescue Mode disabled" : "",
		(be32toh(stats.brdprop) & BOARD_NEEDS_SYS_RST_PULL_UP) ? ", ARM AP" : ", x86 AP"
	);

	return 0;
}

int get_headerinfo(const char *path)
{
	uint32_t size, entry_point, offset = 0;
	void *data = read_file(path, &size);
	struct SignedHeader *hdr;
	struct SignedManifest *mf;

	do {
		hdr = (struct SignedHeader *)(data + offset);
		mf = (struct SignedManifest *)(data + offset);

		if (!valid_signed_header(hdr) && !valid_signed_manifest(mf))
			goto proceed;

		if (hdr->magic == MAGIC_HAVEN || hdr->magic == MAGIC_CITADEL || hdr->magic == MAGIC_DAUNTLESS) {
			// We need this for base_to_image
			switch (hdr->magic) {
				case MAGIC_HAVEN:
					firmware = GSC_DEVICE_H1;
					break;
				case MAGIC_CITADEL:
					firmware = GSC_DEVICE_CT;
					break;
				case MAGIC_DAUNTLESS:
					firmware = GSC_DEVICE_DT;
					break;
			}

			// If a section is specified, only show it.
			if (fbool("section")) {
				if (strcmp(base_to_image(hdr->ro_base), fval("section")))
					goto proceed;
			}

			// Incrediby janky, but it works, so I can't complain
			entry_point = (uint32_t)*(uint32_t *)(data + offset + sizeof(struct SignedHeader) + 4) - hdr->rx_base;
			entry_point = (entry_point & 1) ? entry_point ^ 1 : entry_point; // XOR out the ARM thumb bit if present.

			printf("GSC Firmware (Image Offset 0x%x)\n", offset);
			printf("  \e[1;31mMagic Value \e[0m: 0x%x ", hdr->magic);
			printf((hdr->magic == MAGIC_HAVEN) ? "(Haven)\n" : (hdr->magic == MAGIC_DAUNTLESS) ? "(Dauntless)\n" : (hdr->magic == MAGIC_CITADEL) ? "(Citadel)\n" : "(Unknown)\n");
			printf("  \e[1;31mImage Size\e[0m: 0x%x\n", hdr->image_size);
			printf("  \e[1;31mBase\e[0m: RO 0x%x, RX 0x%x (%s)\n", hdr->ro_base, hdr->rx_base, base_to_image(hdr->ro_base));
			printf("  \e[1;31mMax\e[0m: RO 0x%x, RX 0x%x\n", hdr->ro_max, hdr->rx_max);
			printf("  \e[1;31mKey ID\e[0m: 0x%x\n", hdr->keyid);
			// Don't show DEV ID's for non-node-locked images and images that didn't yet support it.
			if (hdr->dev_id0_ != SIGNED_HEADER_PADDING && hdr->dev_id0_ != 0) 
				printf("  \e[1;31mDev IDs\e[0m: 0x%x, 0x%x\n", hdr->dev_id0_, hdr->dev_id1_);
			printf("  \e[1;31mVersion\e[0m: %d.%d.%d\n", hdr->epoch_, hdr->major_, hdr->minor_);
			printf("  \e[1;31mSignature Timestamp\e[0m: (0x%lx) %s", hdr->timestamp_, ctime(&hdr->timestamp_));
			printf("  \e[1;31mBoard ID\e[0m: %08x:%08x:%08x\n", SIGNED_HEADER_PADDING ^ hdr->board_id.id, SIGNED_HEADER_PADDING ^ hdr->board_id.mask, SIGNED_HEADER_PADDING ^ hdr->board_id.flags);
			printf("  \e[1;31mConfig1\e[0m: 0x%x\n", hdr->config1_);
			printf("  \e[1;31mEntry Point Offset\e[0m: 0x%x (0x%x)\n", entry_point, entry_point + hdr->rx_base);
			printf("  \e[1;31mChecksums\e[0m: Himg =%X... Hfss =%X... Hinf =%X...\n", hdr->img_chk_, hdr->fuses_chk_, hdr->info_chk_);
		}else if (mf->identifier == ID_ROM_EXT || mf->identifier == ID_OWNER_FW) {
			printf("OpenTitan GSC Firmware (Image Offset 0x%x)\n", offset);
			printf("  \e[1;31mIdentifier\e[0m: 0x%x", mf->identifier);
			printf((mf->identifier == ID_ROM_EXT) ? " (ROM Extension)\n" : (mf->identifier == ID_OWNER_FW) ? " (Owner Firmware)\n" : " (Unknown)\n");
			printf("  \e[1;31mAddress Translation\e[0m: 0x%x\n", mf->address_translation);
			printf("  \e[1;31mManifest Version\e[0m: 0.%d.%d\n", mf->manifest_major, mf->manifest_minor);
			printf("  \e[1;31mCode\e[0m: Start 0x%x, End 0x%x, Version %d.%d.%d\n", 
				mf->code_start, mf->code_end, mf->security_version, mf->major, mf->minor);
			printf("  \e[1;31mEntry Point Offset\e[0m: 0x%x\n", mf->entry_point);
			printf("  \e[1;31mLength\e[0m: 0x%x\n", mf->image_size);
			printf("  \e[1;31mSigned Region End\e[0m: 0x%x\n", mf->signed_region_end);
			printf("  \e[1;31mKey\e[0m: ID 0x%x, Max Version 0x%x\n", bswap(mf->key[0]), mf->max_key_version);
			printf("  \e[1;31mSignature Timestamp\e[0m: (0x%lx) %s", mf->timestamp, ctime(&mf->timestamp));
		}

		proceed:
		offset = round_up_2kb(offset + 1);
	} while (offset < size); // Only show half the image if --verbose isn't passed.

	return 0;
}

static void print_hex32(uint32_t *data, size_t len, bool carray)
{
	int i;

	if (!carray) {
		for (i = 0; i < len; ++i)
			printf("%08x ", data[i]);
		printf("\n");
	}else{
		printf("static const uint32_t DATA[%lu] = {", len);
		for (i = 0; i < len; ++i) {
			if ((i % 6) == 0) printf("\n\t");
			printf("0x%08x, ", data[i]);
		}
		printf("\n};\n");
	}
}

int get_keyinfo(const char *path)
{
	uint32_t size, offset = 0;
	void *data = read_file(path, &size);
	struct SignedHeader *hdr;
	struct SignedManifest *mf;
	
	do {
		hdr = (struct SignedHeader *)(data + offset);
		mf = (struct SignedManifest *)(data + offset);

		if (!valid_signed_header(hdr) && !valid_signed_manifest(mf))
			goto proceed;

		if (hdr->magic == MAGIC_HAVEN || hdr->magic == MAGIC_CITADEL || hdr->magic == MAGIC_DAUNTLESS) {
			printf("\e[1;32mGSC Firmware\e[0m (Image Offset 0x%x)\n", offset);

			printf("  \e[1;31mRSA Public Key\e[0m: \n");
			print_hex32(&hdr->keyid, ARRAY_LEN(hdr->key) + 1, true);
			printf("  \e[1;31mRSA Montgomery Signature\e[0m: \n");
			print_hex32(hdr->signature, ARRAY_LEN(hdr->signature), true);
			printf("  \e[1;31mFuse Map\e[0m: ");
			print_hex32(hdr->fusemap, ARRAY_LEN(hdr->fusemap), false);
			printf("  \e[1;31mInfo Map\e[0m: ");
			print_hex32(hdr->infomap, ARRAY_LEN(hdr->infomap), false);
			
			printf("\n");
		}else if (mf->identifier == ID_ROM_EXT || mf->identifier == ID_OWNER_FW) {
			printf("\e[1;32mOpenTitan Firmware\e[0m (Image Offset 0x%x)\n", offset);

			printf("  \e[1;31mPublic Key\e[0m: \n");
			print_hex32(mf->key, ARRAY_LEN(mf->key), true);
			printf("  \e[1;31mSignature\e[0m: \n");
			print_hex32(mf->signature, ARRAY_LEN(mf->signature), true);
		}

		proceed:
		offset = round_up_2kb(offset + 1);
	} while (offset < size);

	return 0;
}