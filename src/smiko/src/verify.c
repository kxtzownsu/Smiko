/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "args.h"
#include "chip_config.h"
#include "common.h"
#include "bn.h"
#include "signed_header.h"
#include "trunks.h"
#include "verify.h"

/* The following is a large list of every known GSC firmware public key.
 * Each key is used in the RSA verification of a given GSC image. If a key is
 * not provided in this list, the header modulus will be used with an assumed
 * exponent of 3. 
 *
 * Note: Under normal circumstances, no GSC will ever use the modulus in the header!
 *	   If you've self-signed an image, it reporting valid here does NOT mean it will
 *	   report valid on the target GSC! ~ Hannah
 *
 * Note: Only public keys used in publicly released images should be added here!
 *	   No keys for testing should be added! ~ Hannah
 */
#include "gsc_keys.h"

/* Data space containing the expected hash. */
static uint32_t SB_BL_SIG[SHA256_DIGEST_WORDS];
/* Data space containing the warmboot hash. */
static uint32_t last_hash[SHA256_DIGEST_WORDS];


int sb_comp_status_sig_match = 0;
/* Hardcoded hash of buf that if passed to SB_BL_SIG unlocks for execution. */
static const uint32_t expected_hash[8] = {
	0xE303EC7A, 0x68A03A27, 0xDD18053E, 0x39F8DBBD,
	0x9B553578, 0xB4598244, 0xC59F62D1, 0x61B8509E,
};
/* Hardcoded hash to be XOR'd with the hash of buf and hash  */
const uint32_t warmboot_hash[SHA256_DIGEST_WORDS] = {
    0xad2d98d3, 0xa95acac6, 0x0d247c19, 0xb5f0a100, 
    0x12044f20, 0xa8c2f5fd, 0x7f9bb2ce, 0xaab9ab98,
};

/* ========== SignedHeader Verification Functions ========== */

bool valid_signed_header(const struct SignedHeader *header)
{
	if (header->magic != MAGIC_HAVEN && 
		header->magic != MAGIC_CITADEL &&
		header->magic != MAGIC_DAUNTLESS)
		return false;

	if (header->image_size < 0x800)
		return false;

	// Reject Dauntless cryptolib headers.
	if (header->image_size & 0x50000000)
		return false;

	/*
	 * Both Rx base and Ro base are the memory mapped address, but they
	 * should have the same offset. The rx section starts after the header.
	 */
	if (header->rx_base != header->ro_base + sizeof(struct SignedHeader))
		return false;

	return true;
}

bool valid_signed_manifest(const struct SignedManifest *manifest)
{
	if (manifest->identifier != ID_ROM_EXT && manifest->identifier != ID_OWNER_FW)
		return false;

	if (manifest->code_start > manifest->code_end)
		return false;

	if (manifest->code_start > manifest->image_size)
		return false;

	if (manifest->code_end > manifest->image_size)
		return false;

	if (manifest->entry_point > manifest->code_end)
		return false;

	if (manifest->entry_point < manifest->code_start)
		return false;

	return true;
}

/* Rounds and address up to the next 2KB boundary if not one already */
uint32_t round_up_2kb(const uint32_t addr)
{
	const uint32_t mask = (2 * 1024) - 1;

	return (addr + mask) & ~mask;
}

/* Returns the RW header or -1 if one cannot be found */
uint32_t find_rw_header(const void *data, size_t size, uint32_t offset)
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

/* ========== SignedHeader RSA Verification Functions ========== */
/* Written 90% by Evelyn, thank you so much! */


/* Array of various e-fuses used by the GSC. */
static uint32_t efuses[FUSE_MAX]; 

void prepare_efuses(struct SignedHeader *hdr)
{
	/* Wipe the entire E-Fuses range. This should (somewhat) prevent
	 * fuses we don't define here from being random.
	 */
	memset(efuses, 0x55, sizeof(efuses));
	
	/* Prepare the E-Fuses array based on the magic value used
	 * in signing. Logical offsets are pulled from the signing impl's
	 * fuses.xml for each individual chip. 
	 */
	switch (hdr->magic) {
		case MAGIC_HAVEN:
			/* Note: Only B2 is supported! A1 and B1 currently aren't detectable. ~ Hannah */
			efuses[0x06] = hdr->dev_id0_; // DEV_ID0
			efuses[0x07] = hdr->dev_id1_; // DEV_ID1
			efuses[0x43] = 0x55555555;    // FLASH_PERSO_PAGE_LOCK
			efuses[0x47] = 0x55555502;    // FW_DEFINED_DATA_BLK0
			efuses[0x7d] = 0x55555540;    // FW_DEFINED_DATA_EXTRA_BLK6
			efuses[0x55] = 0x55555137;    // FW_DEFINED_BROM_APPLYSEC
			break;
		case MAGIC_CITADEL:
			// TODO (Evelyn): Bruteforce the Citadel efuses
			efuses[0x14] = hdr->dev_id0_; // DEV_ID0
			efuses[0x18] = hdr->dev_id1_; // DEV_ID1
			break;
		case MAGIC_DAUNTLESS:
			efuses[0x08] = 0x55555542;
			efuses[0x35] = 0x55555550;
			efuses[0x3e] = 0x55555540;
			break;
		default:
			fprintf(stderr, "Error: Unrecognized chipset, cannot populate E-Fuses.\n");
			break;
	}
}

/* Array of rollback bits used by the GSC. */
static uint32_t info_space[INFO_MAX]; 

void prepare_info(struct SignedHeader *hdr, uint32_t *info)
{
	/* TODO (Hannah): Populate INFO based on image header? */
	memset(info, 0xff, INFO_MAX * 4);
}

static void sigUnlock(void)
{
	sb_comp_status_sig_match = 
		!memcmp(SB_BL_SIG, expected_hash, SHA256_DIGEST_LENGTH) || // Hardcoded RSA buf hash
		!memcmp(SB_BL_SIG, last_hash, SHA256_DIGEST_LENGTH); // Warmboot hash
}

static int unlockedForExecution(void)
{
	return sb_comp_status_sig_match & 1;
}

/* "rev r# r#" reimpl from ARM */
uint32_t bswap(uint32_t x)
{
	uint32_t result;

	result = (x >> 24) | ((x >> 8) & 0x0000FF00) | ((x << 8) & 0x00FF0000) | (x << 24);

	return result;
}


const uint32_t *LOADERKEY_find(const struct SignedHeader *h)
{
	/* Haven Keys */
	if (h->keyid == LOADERKEY_H1_ROM_PROD[0])
		return LOADERKEY_H1_ROM_PROD;
	if (h->keyid == LOADERKEY_H1_ROM_DEV[0])
		return LOADERKEY_H1_ROM_DEV;
	if (h->keyid == LOADERKEY_CR50_LOADER_PROD[0])
		return LOADERKEY_CR50_LOADER_PROD;
	if (h->keyid == LOADERKEY_CR50_LOADER_NODELOCKED[0])
		return LOADERKEY_CR50_LOADER_NODELOCKED;
	if (h->keyid == LOADERKEY_CR50_LOADER_TAST_TEST_0_0_11[0])
		return LOADERKEY_CR50_LOADER_TAST_TEST_0_0_11;
	if (h->keyid == LOADERKEY_CR50_LOADER_TAST_TEST_0_0_14[0])
		return LOADERKEY_CR50_LOADER_TAST_TEST_0_0_14;
	if (h->keyid == LOADERKEY_CR50_LOADER_DEV[0])
		return LOADERKEY_CR50_LOADER_DEV;

	/* Citadel Keys*/
	if (h->keyid == LOADERKEY_NUGGETOS_ROM_PROD[0])
		return LOADERKEY_NUGGETOS_ROM_PROD;
	if (h->keyid == LOADERKEY_NUGGETOS_ROM_DEV[0])
		return LOADERKEY_NUGGETOS_ROM_DEV;
	if (h->keyid == LOADEREKEY_NUGGETOS_LOADER_PROD[0])
		return LOADEREKEY_NUGGETOS_LOADER_PROD;

	/* Dauntless Keys */
	if (h->keyid == LOADERKEY_TI50_ROM_PROD[0])
		return LOADERKEY_TI50_ROM_PROD;
	if (h->keyid == LOADERKEY_TI50_ROM_DEV[0])
		return LOADERKEY_TI50_ROM_DEV;
	if (h->keyid == LOADERKEY_TI50_LOADER_PROD[0])
		return LOADERKEY_TI50_LOADER_PROD;
	if (h->keyid == LOADERKEY_TI50_LOADER_DEV[0])
		return LOADERKEY_TI50_LOADER_DEV;
	if (h->keyid == LOADERKEY_NUGGETOS_ROM_PROD[0])
		return LOADERKEY_NUGGETOS_ROM_PROD; // Note: This key is shared by the Dauntless and Citadel BootROM! ~ Hannah
	if (h->keyid == LOADERKEY_NUGGETOS_ROM_DEV[0])
		return LOADERKEY_NUGGETOS_ROM_DEV; // Note: This key is shared by the Dauntless and Citadel BootROM! ~ Hannah
	if (h->keyid == LOADERKEY_ACROPORA_LOADER_PROD[0])
		return LOADERKEY_ACROPORA_LOADER_PROD;
	if (h->keyid == LOADERKEY_ACROPORA_LOADER_DEV[0])
		return LOADERKEY_ACROPORA_LOADER_DEV;

	/* No public key found; use the modulus in the header. */
	return &h->keyid;
}

static uint32_t buf[RSA_NUM_WORDS];
static uint32_t hash[SHA256_DIGEST_WORDS];

static void LOADERKEY_verify(const uint32_t *key, const uint32_t *signature,
							const uint32_t *sha256)
{
	uint32_t step, offset;
	int i;

	LOADERKEY_modpow(key, signature, buf);

	/*
	 * If key was not 3Kb, assume 2Kb and expand for subsequent
	 * padding + hash verification mangling.
	 */
	if (key[96] == 0) {
		buf[95] ^= buf[63];
		buf[63] ^= 0x1ffff;
		for (i = 63; i < 95; ++i)
			buf[i] ^= -1;
	}

	/*
	 * XOR in offsets across buf. Mostly to get rid of all those -1 words
	 * in there.
	 */
	offset = rand() % RSA_NUM_WORDS;
	step = (RANDOM_STEP % RSA_NUM_WORDS) ?: 1;

	for (i = 0; i < RSA_NUM_WORDS; ++i) {
		buf[offset] ^= (0x1000u + offset);
		offset = (offset + step) % RSA_NUM_WORDS;
	}

	/*
	 * Xor digest location, so all words becomes 0 only iff equal.
	 *
	 * Also XOR in offset and non-zero const. This to avoid repeat
	 * glitches to zero be able to produce the right result.
	 */
	offset = rand() % SHA256_DIGEST_WORDS;
	step = (RANDOM_STEP % SHA256_DIGEST_WORDS) || 1;
	for (i = 0; i < SHA256_DIGEST_WORDS; ++i) {
		if (!fbool("skip_hash_checking")) {
			buf[offset] ^= bswap(sha256[SHA256_DIGEST_WORDS - 1 - offset])
				^ (offset + 0x10u);
		}else{
			buf[offset] = 0x1010;
		}
		offset = (offset + step) % SHA256_DIGEST_WORDS;
	}

	/* Hash the resulting buffer. */
	SHA256((const uint8_t *)buf, RSA_NUM_BYTES, hash);

	for (i = 0; i < SHA256_DIGEST_WORDS; ++i)
		SB_BL_SIG[i] = hash[i];

	/* Unlock attempt. */
	sigUnlock();

	if (fbool("verbose") && !unlockedForExecution() || fbool("debug_buf")) {
		printf("Debug: Calculated buf: ");
		for (i = 0; i < RSA_NUM_WORDS; ++i)
			printf("%08x ", buf[i]);
		printf("\n");
	}
}

static void LOADERKEY_seed_warmboot(uint32_t *ladder)
{
	uint32_t bits, bitshift;
	int offset, i;
	SHA256_CTX ctx;

	for (i = 0; i < SHA256_DIGEST_WORDS; ++i)
		ladder[i] = rand();

	SHA256_Init(&ctx);
	SHA256_Update(&ctx, buf, RSA_NUM_BYTES);
	SHA256_Update(&ctx, hash, SHA256_DIGEST_LENGTH);
	SHA256_Final(ladder, &ctx);

	for (i = 0; i < SHA256_DIGEST_WORDS; ++i)
		bits |= ladder[i] ^ warmboot_hash[i];

	for (bitshift = bits; bitshift >>= 1; bitshift != 0)
		offset++;

	for (i = 0; i < SHA256_DIGEST_WORDS; ++i)
		ladder[i] = rand() & (0 - (32 - offset) >> 5);
}

void hash_efuses(struct SignedHeader *hdr, uint32_t *out)
{
	static uint32_t fuses[FUSE_MAX];
	int i;
	
	// Prepare E-Fuses for the target header.
	prepare_efuses(hdr);

	/* Sense fuses into RAM array; hash array. */
	for (i = 0; i < FUSE_MAX; ++i)
		fuses[i] = (hdr->magic != MAGIC_HAVEN) ? (hdr->magic != MAGIC_CITADEL) ? FUSE_IGNORE_D : FUSE_IGNORE_C : FUSE_IGNORE_B;

	for (i = 0; i < FUSE_MAX; ++i) {
		/*
		* For the fuses the header cares about, read their values
		* into the map.
		*/
		if (hdr->fusemap[i>>5] & (1 << (i&31))) {
			/*
			* BNK0_INTG_CHKSUM is the first fuse and as such the
			* best reference to the base address of the fuse
			* memory map.
			*/
			fuses[i] = efuses[i];
		}
	}

	/* If this is a node locked image, copy the DEV_IDs into our fuses. */
	if (hdr->dev_id0_ && hdr->dev_id0_ != SIGNED_HEADER_PADDING)
		fuses[FUSE_MAX - 2] = hdr->dev_id0_;
	if (hdr->dev_id1_ && hdr->dev_id1_ != SIGNED_HEADER_PADDING)
		fuses[FUSE_MAX - 2] = hdr->dev_id1_;

	SHA256((uint8_t *) fuses, sizeof(fuses), out);
}

void hash_info(struct SignedHeader *hdr, uint32_t *out, uint32_t *info_bank)
{
	static uint32_t info[INFO_MAX];
	int i;

	prepare_info(hdr, info_bank);

	/* Sense info into RAM array; hash array. */
	for (i = 0; i < INFO_MAX; ++i)
		info[i] = (hdr->magic != MAGIC_HAVEN) ? (hdr->magic != MAGIC_CITADEL) ? INFO_IGNORE_D : INFO_IGNORE_C : INFO_IGNORE_B;

	for (i = 0; i < INFO_MAX; ++i) {
		if (hdr->infomap[i>>5] & (1 << (i&31))) {
			int retval = 0;

			/* Read the target info bank. */
			info[i] ^= info_bank[i] ^ retval;
		}
	}
	
	SHA256((uint8_t *) info, sizeof(info), out);
}

void verify_image(void *data, uint32_t data_size)
{
	static struct {
		uint32_t img_hash[SHA256_DIGEST_WORDS];
		uint32_t fuses_hash[SHA256_DIGEST_WORDS];
		uint32_t info_hash[SHA256_DIGEST_WORDS];
	} hashes;
	static uint32_t final_hash[SHA256_DIGEST_WORDS];
	static uint32_t ladder[SHA256_DIGEST_WORDS];
	uint32_t offset = 0;
	const uint32_t *pubkey;
	int i;
	struct SignedHeader *hdr;
	struct SignedManifest *mf;

	/* We only generate this ladder once. */
	LOADERKEY_seed_warmboot(ladder);

	do {
		hdr = (struct SignedHeader *)(data + offset);
		mf = (struct SignedManifest *)(data + offset);
		

		if (!valid_signed_header(hdr) && !valid_signed_manifest(mf))
			goto proceed;

		memset(hash, 0, SHA256_DIGEST_LENGTH);

		if (hdr->magic == MAGIC_HAVEN || hdr->magic == MAGIC_CITADEL || hdr->magic == MAGIC_DAUNTLESS) {
			/* TODO (Hannah/Evelyn): Do we want to check the RO/RX ranges per chip here? */

			/* Calculate our header hashes. */
			SHA256((uint8_t *) &hdr->tag,
					hdr->image_size - offsetof(struct SignedHeader, tag),
					(uint8_t *) hashes.img_hash);
			hash_efuses(hdr, (uint8_t *) hashes.fuses_hash);
			hash_info(hdr, (uint8_t *) hashes.info_hash, info_space);

			if (fbool("verbose")) {
				printf("Debug: Header hashes:\n");
				printf("Himg =%08X..%08X : %d\n", hashes.img_hash[0], hashes.img_hash[7], hashes.img_hash[0] == hdr->img_chk_);
				printf("Hfss =%08X..%08X : %d\n", hashes.fuses_hash[0], hashes.fuses_hash[7], hashes.fuses_hash[0] == hdr->fuses_chk_);
				printf("Hinf =%08X..%08X : %d\n", hashes.info_hash[0], hashes.info_hash[7], hashes.info_hash[0] == hdr->info_chk_);
			}

			/* Hash our set of hashes to get final hash. */
			SHA256((uint8_t *) &hashes, sizeof(hashes),
					(uint8_t *) final_hash);
			
			if (!fbool("skip_hash_checking")) {
				/* Verify the hash checksums. */
				if (hdr->img_chk_ != hashes.img_hash[0] || hdr->fuses_chk_ != hashes.fuses_hash[0] || hdr->info_chk_ != hashes.info_hash[0]) {
					printf("Info: Image at offset 0x%x has invalid checksums.\n", offset);
					goto proceed;
				}
			}

			/* Write measured hash to unlock buffer. */
			for (i = 0; i < SHA256_DIGEST_WORDS; ++i)
				SB_BL_SIG[i] = final_hash[i] ^ ladder[i];

			/* Unlock attempt. */
			sigUnlock();

			if (!unlockedForExecution()) {
				pubkey = LOADERKEY_find(hdr);
				LOADERKEY_verify(pubkey, &hdr->signature, &final_hash);

				if (!unlockedForExecution()) {
					for (i = 0; i < SHA256_DIGEST_WORDS; ++i)
						last_hash[i] = final_hash[i];
				}
			}
		}else if (mf->identifier == ID_ROM_EXT || mf->identifier == ID_OWNER_FW) {
			/* TODO (Hannah/Evelyn): Implement a proper RSA key check for OpenTitan? */
		}

		if (unlockedForExecution()) {
			printf("Info: Image at offset 0x%x is verified.\n", offset);
		}else{
			fprintf(stderr, "Info: Image at offset 0x%x contains an invalid signature.\n", offset);
		}


		proceed:
		offset = round_up_2kb(offset + 1);
		continue;
	} while (offset < data_size);
}

void repack_image_sig(void *data, size_t len)
{
	
}
