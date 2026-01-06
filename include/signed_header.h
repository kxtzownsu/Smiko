/* Copyright 2015 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 * Reorganized and improved by Hannah.
 */
#ifndef __CROS_EC_SIGNED_HEADER_H
#define __CROS_EC_SIGNED_HEADER_H
#include <stdbool.h>
#include <stdint.h>

#define FUSE_MAX 128             /* baked in rom! */
#define FUSE_PADDING 0x55555555  /* baked in hw! */
#define INFO_MAX 128             /* baked in rom! */


// Haven chips
#define FUSE_IGNORE_B 0xa3badaac  // baked in rom!
#define INFO_IGNORE_B 0xaa3c55c3  // baked in rom!
// Citadel chips
#define FUSE_IGNORE_C 0x3aabadac  // baked in rom!
#define INFO_IGNORE_C 0xa5c35a3c  // baked in rom!
// Dauntless chips
#define FUSE_IGNORE_D 0xdaa3baca  // baked in rom!
#define INFO_IGNORE_D 0x5a3ca5c3  // baked in rom!


/* Dauntless and Haven headers use different magic values */
#define MAGIC_HAVEN 0xFFFFFFFF
#define MAGIC_CITADEL 0xFFFFFFFE
#define MAGIC_DAUNTLESS 0xFFFFFFFD

/* Default value for _pad[] words */
#define SIGNED_HEADER_PADDING 0x33333333

typedef struct SignedHeader {
#ifdef __cplusplus
	SignedHeader() : 
		magic(MAGIC_HAVEN), 
		image_size(0), 
		epoch_(0x1337), 
		major_(0), 
		minor_(0xbabe),
		p4cl_(0), 
		applysec_(0), 
		config1_(0), 
		err_response_(0), 
		expect_response_(0),
		swap_mark({0, 0}),
		dev_id0_(0),
		dev_id1_(1)
	{
		memset(signature, 'S', sizeof(signature));
		memset(tag, 'T', sizeof(tag));
		memset(fusemap, 0, sizeof(fusemap));
		memset(infomap, 0, sizeof(infomap));
		memset(&_pad, SIGNED_HEADER_PADDING, sizeof(_pad));
		// Below all evolved out of _pad, thus must also be initialized to '3'
		// for backward compatibility.
		memset(&rw_product_family_, SIGNED_HEADER_PADDING,
			sizeof(rw_product_family_));
		memset(&u, SIGNED_HEADER_PADDING, sizeof(u));
		memset(&board_id, SIGNED_HEADER_PADDING, sizeof(board_id));
	}

	void markFuse(uint32_t n)
	{
		fusemap[n / 32] |= 1 << (n & 31);
	}

	void markInfo(uint32_t n) 
	{
		infomap[n / 32] |= 1 << (n & 31);
	}

	static uint32_t fuseIgnore(bool c, bool d) {
		return d ? FUSE_IGNORE_D : c ? FUSE_IGNORE_C : FUSE_IGNORE_B;
	}
	static uint32_t infoIgnore(bool c, bool d) {
		return d ? INFO_IGNORE_D : c ? INFO_IGNORE_C : INFO_IGNORE_B;
	}

	bool plausible() const {
		switch (magic) {
			case MAGIC_HAVEN:
			case MAGIC_CITADEL:
			case MAGIC_DAUNTLESS:
				break;
			default:
				return false;
		}
		
		if (keyid == -1u) return false;
		if (ro_base >= ro_max) return false;
		if (rx_base >= rx_max) return false;
		if (_pad[0] != SIGNED_HEADER_PADDING) return false;
		return true;
	}
#endif /* __cplusplus */

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
		// 2nd FIPS signature (cr51/cr52 RW)
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
	uint32_t rw_product_family_; // 0 == PRODUCT_FAMILY_ANY
                                 // Stored as (^SIGNED_HEADER_PADDING)
                                 // TODO(ntaha): add reference to product family
                                 // enum when available.
	/* Board ID type, mask, flags (stored ^SIGNED_HEADER_PADDING, ignored in RO firmware updates) */
	struct {
		uint32_t id;
		uint32_t mask;
		uint32_t flags;
	} board_id;

	uint32_t dev_id0_;    /* node id, if locked */
	uint32_t dev_id1_;
	uint32_t fuses_chk_;  /* top 32 bit of expected fuses hash */
	uint32_t info_chk_;   /* top 32 bit of expected info hash */
} SignedHeader;

#define TOP_IMAGE_SIZE_BIT (1 <<			\
	    (sizeof(((struct SignedHeader *)0)->image_size) * 8 - 1))

/*
 * It is a mere convention, but all prod keys are required to have key IDs
 * such, that bit D2 is set, and all dev keys are required to have key IDs
 * such, that bit D2 is not set.
 *
 * This convention is enforced at the key generation time.
 */
#define G_SIGNED_FOR_PROD(h) ((h)->keyid & BIT(2))


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

/* Header used by OpenTitan images. See src/ti50/third_party/lowRISC/opentitan/sw/device/silicon_creator/lib/manifest.h */
struct SignedManifest {
	uint32_t signature[96]; /* RSA-3072 or ECDSA SHA256 signature */
	/* --------------------- everything below is signed */
	uint32_t constraint_selector_bits;
	uint32_t constraint_device_id[8];
	uint32_t constraint_manuf_state_creator;
	uint32_t constraint_manuf_state_owner;
	uint32_t constraint_life_cycle_state;
	uint32_t key[96];     /* Modulus of the public key used in signing */
	uint32_t address_translation;
	uint32_t identifier;  /* ID_ROM_EXT or ID_OWNER_FW */
	uint16_t manifest_major; /* Must be kManifestVersionMajor2 for ROM */
	uint16_t manifest_minor;
	uint32_t signed_region_end;
	uint32_t image_size;  /* Length of the image minus the signature */
	uint32_t major;
	uint32_t minor;
	uint32_t security_version; /* Security version for anti-rollback */
	uint64_t timestamp;   /* Time of signing */
	uint32_t binding_value[8];
	uint32_t max_key_version;
	uint32_t code_start;
	uint32_t code_end;
	uint32_t entry_point; /* Offset in the code of the entry function */
	uint32_t extensions[30]; /* Extra manifest space for free use */
};
#endif