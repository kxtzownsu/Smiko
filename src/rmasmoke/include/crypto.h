/* Hannah's shitty GSCTool crypto implementation!
 * Most of it's intended for Cr50/Ti50 firmware upgrades
 * and image signing, but other than that it provides some
 * simple functions for convenience.
*/

#ifndef __SMIKO_CRYPTO_H
#define __SMIKO_CRYPTO_H
#define SHA256_DIGEST_WORDS (SHA256_DIGEST_LENGTH / sizeof(uint32_t))
#define MIN(a, b) \
({ \
    __typeof__(a) temp_a = (a);	\
    __typeof__(b) temp_b = (b); \
    \
    temp_a < temp_b ? temp_a : temp_b; \
})

unsigned int calculate_block_digest(const void *data, size_t data_size, 
                                    uint32_t offset, bool use_sha256);

// Quick and dirty libcrypto wrapper designed to emulate DCRYPTO_SHA256_hash
uint8_t *CRYPTO_SHA256_hash(const void *data, uint32_t n,
	uint8_t *digest);
#endif /* __SMIKO_CRYPTO_H */