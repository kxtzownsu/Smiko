/* Copyright 2015 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef __CROS_EC_SIGNED_HEADER_H
#define __CROS_EC_SIGNED_HEADER_H
#include <stdbool.h>
#include <stdint.h>

#define FUSE_PADDING 0x55555555  /* baked in hw! */
#define FUSE_IGNORE 0xa3badaac   /* baked in rom! */
#define FUSE_IGNORE_DT 0xdaa3baca   /* baked in rom! */
#define FUSE_MAX 128             /* baked in rom! */

#define INFO_MAX 128             /* baked in rom! */
#define INFO_IGNORE 0xaa3c55c3   /* baked in rom! */
#define INFO_IGNORE_DT 0 // I need to fix this later

/* Dauntless and Haven headers use different magic values */
#define MAGIC_HAVEN 0xFFFFFFFF
#define MAGIC_DAUNTLESS 0xFFFFFFFD

/* Default value for _pad[] words */
#define SIGNED_HEADER_PADDING 0x33333333

struct SignedHeader {
	uint32_t magic;       /* -1 (thanks, boot_sys!) */
	uint32_t signature[96];
	uint32_t img_chk_;    /* top 32 bit of expected img_hash */
	/* --------------------- everything below is part of img_hash */
	uint32_t tag[7];      /* words 0-6 of RWR/FWR */
	uint32_t keyid;       /* word 7 of RWR */
	uint32_t key[96];     /* public key to verify signature with */
	uint32_t image_size;
	uint32_t ro_base;     /* readonly region */
	uint32_t ro_max;
	uint32_t rx_base;     /* executable region */
	uint32_t rx_max;
	uint32_t fusemap[FUSE_MAX / (8 * sizeof(uint32_t))];
	uint32_t infomap[INFO_MAX / (8 * sizeof(uint32_t))];
	uint32_t epoch_;      /* word 7 of FWR */
	uint32_t major_;      /* keyladder count */
	uint32_t minor_;
	uint64_t timestamp_;  /* time of signing */
	uint32_t p4cl_;
	/* bits to and with FUSE_FW_DEFINED_BROM_APPLYSEC */
	uint32_t applysec_;
	/* bits to mesh with FUSE_FW_DEFINED_BROM_CONFIG1 */
	uint32_t config1_;
	/* bits to or with FUSE_FW_DEFINED_BROM_ERR_RESPONSE */
	uint32_t err_response_;
	/* action to take when expectation is violated */
	uint32_t expect_response_;

	union {
		// 2nd FIPS signature (gnubby RW / Cr51)
		struct {
			uint32_t keyid;
			uint32_t r[8];
			uint32_t s[8];
		} ext_sig;

		// FLASH trim override (Dauntless RO)
		// iff config1_ & 65536
		struct {
			uint32_t FSH_SMW_SETTING_OPTION3;
			uint32_t FSH_SMW_SETTING_OPTION2;
			uint32_t FSH_SMW_SETTING_OPTIONA;
			uint32_t FSH_SMW_SETTING_OPTIONB;
			uint32_t FSH_SMW_SMP_WHV_OPTION1;
			uint32_t FSH_SMW_SMP_WHV_OPTION0;
			uint32_t FSH_SMW_SME_WHV_OPTION1;
			uint32_t FSH_SMW_SME_WHV_OPTION0;
		} fsh;
	} u;

	/* Padding to bring the total structure size to 1K. */
	uint32_t _pad[5];
	struct {
		unsigned size:12;
		unsigned offset:20;
	} swap_mark;

	/* Field for managing updates between RW product families. */
	uint32_t rw_product_family_;
	/* Board ID type, mask, flags (stored ^SIGNED_HEADER_PADDING, ignored in RO firmware updates) */
	uint32_t board_id_type;
	uint32_t board_id_type_mask;
	uint32_t board_id_flags;

	uint32_t dev_id0_;    /* node id, if locked */
	uint32_t dev_id1_;
	uint32_t fuses_chk_;  /* top 32 bit of expected fuses hash */
	uint32_t info_chk_;   /* top 32 bit of expected info hash */
};

// Version information from SignedHeader
struct signed_header_version {
	uint32_t minor;
	uint32_t major;
	uint32_t epoch;
};

/* Identifier that marks the header as ROM_EXT "OTRE" */
#define ID_ROM_EXT 0x4552544F
/* Identifier that marks the header as owner firmware "OTB0" */
#define ID_OWNER_FW 0x3042544F

/* Header used by OpenTitan images. See src/third_party/lowriscv/opentitan/sw/device/silicon_creator/lib/manifest.h */
struct SignedManifest {
	uint32_t signature[96];
	uint32_t constraint_selector_bits;
	uint32_t constraint_device_id[8];
	uint32_t constraint_manuf_state_creator;
	uint32_t constraint_manuf_state_owner;
	uint32_t constraint_life_cycle_state;
	uint32_t modulus[96];
	uint32_t address_translation;
	uint32_t identifier;
	uint16_t manifest_version_major;
	uint16_t manifest_version_minor;
	uint32_t signed_region_end;
	uint32_t length;
	uint32_t version_major;
	uint32_t version_minor;
	uint32_t security_version;
	uint32_t timestamp_low;
	uint32_t timestamp_high;
	uint32_t binding_value[8];
	uint32_t max_key_version;
	uint32_t code_start;
	uint32_t code_end;
	uint32_t entry_point;
	uint32_t extensions[30];
};

static bool valid_signed_header(const struct SignedHeader *header)
{
	/* Only H1 and D2 are currently supported. */
	if (header->magic != MAGIC_HAVEN && header->magic != MAGIC_DAUNTLESS)
		return false;

	if (header->image_size < 0x800)
		return false;

	/*
	 * Both Rx base and Ro base are the memory mapped address, but they
	 * should have the same offset. The rx section starts after the header.
	 */
	if (header->rx_base != header->ro_base + sizeof(struct SignedHeader))
		return false;

	return true;
}

static bool valid_signed_manifest(const struct SignedManifest *manifest)
{
	if (manifest->identifier != ID_ROM_EXT && manifest->identifier != ID_OWNER_FW)
		return false;

	if (manifest->code_start > manifest->code_end)
		return false;

	if (manifest->code_start > manifest->length)
		return false;

	if (manifest->code_end > manifest->length)
		return false;

	if (manifest->entry_point > manifest->code_end)
		return false;

	if (manifest->entry_point < manifest->code_start)
		return false;

	return true;
}

/* Rounds and address up to the next 2KB boundary if not one already */
static inline uint32_t round_up_2kb(const uint32_t addr)
{
	const uint32_t mask = (2 * 1024) - 1;

	return (addr + mask) & ~mask;
}

/* Returns the RW header or -1 if one cannot be found */
static int32_t find_rw_header(const void *data, size_t size, uint32_t offset)
{
	offset = round_up_2kb(offset);

	while (offset < size) {
		if (valid_signed_header((const struct SignedHeader *)(data + offset)))
			return offset;
		if (valid_signed_manifest((const struct SignedManifest *)(data + offset)))
			return offset;
		offset = round_up_2kb(offset + 1);
	}

	return -1;
}
#endif