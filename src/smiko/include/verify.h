/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SMIKO_INCLUDE_VERIFY_H
#define __SMIKO_INCLUDE_VERIFY_H

#ifdef __cplusplus
extern "C" {
#endif

#define RSA_NUM_WORDS 96
#define RSA_NUM_BYTES (RSA_NUM_WORDS * 4)
#define RANDOM_STEP 5

bool valid_signed_header(const struct SignedHeader *header);

bool valid_signed_manifest(const struct SignedManifest *manifest);

uint32_t round_up_2kb(const uint32_t addr);

/* Returns the RW header or -1 if one cannot be found */
uint32_t find_rw_header(const void *data, size_t size, uint32_t offset);

/* "rev r# r#" reimpl from ARM, pulled from ti50/ports/dauntless/loader/verify.c */
uint32_t bswap(uint32_t x);

const uint32_t *LOADERKEY_find(const struct SignedHeader *h);

void verify_image(void *data, uint32_t data_size);

void repack_image_sig(void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __SMIKO_INCLUDE_VERIFY_H */