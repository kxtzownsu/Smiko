/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SRC_SMIKO_INCLUDE_RSA_H
#define __SRC_SMIKO_INCLUDE_RSA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "signed_header.h"

/* Montgomery c[] += a * b[] / R % key. */
void montMulAdd(const uint32_t *key,
			   uint32_t *c, const uint32_t a,
			   const uint32_t *b);

/* Montgomery c[] = a[] * b[] / R % key. */
void montMul(const uint32_t *key,
			uint32_t *c, const uint32_t *a,
			const uint32_t *b);

/* Montgomery c[] = a[] * 1 / R % key. */
void montMul1(const uint32_t *key,
		 uint32_t *c, const uint32_t *a);

/* Attempt to make a perfect cube from message.
 * Return -1 on failure.
 */
int LOADERKEY_invcube(const uint32_t *key,
		const uint32_t *message, uint32_t *out);

/* In-place exponentiation to power % key.
 * Pulled from Cr50 RO 0.0.14 
 */
void LOADERKEY_modpow(const uint32_t *key,
		const uint32_t *signature, uint32_t *out);

const uint32_t *LOADERKEY_find(const struct SignedHeader *h);

#ifdef __cplusplus
}
#endif

#endif /* __SRC_SMIKO_INCLUDE_RSA_H */