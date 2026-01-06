/* Copyright 2017 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bn.h"
#include "signed_header.h"
#include "verify.h"

/* Montgomery c[] += a * b[] / R % key. */
void montMulAdd(const uint32_t *key,
			   uint32_t *c, const uint32_t a,
			   const uint32_t *b)
{
	register uint64_t tmp;
	uint32_t i, A, B, d0;

	{

		tmp = c[0] + (uint64_t)a * b[0];
		A = tmp >> 32;
		d0 = (uint32_t)tmp * *key++;
		tmp = (uint32_t)tmp + (uint64_t)d0 * *key++;
		B = tmp >> 32;
	}

	for (i = 0; i < RSA_NUM_WORDS - 1; ++i) {
		tmp = A + (uint64_t)a * b[i + 1] + c[i + 1];
		A = tmp >> 32;
		tmp = B + (uint64_t)d0 * *key++ + (uint32_t)tmp;
		c[i] = (uint32_t)tmp;
		B = tmp >> 32;
	}

	c[RSA_NUM_WORDS - 1] = A + B;
}

/* Montgomery c[] = a[] * b[] / R % key. */
void montMul(const uint32_t *key,
			uint32_t *c, const uint32_t *a,
			const uint32_t *b)
{
	int i;

	for (i = 0; i < RSA_NUM_WORDS; ++i)
		c[i] = 0;

	for (i = 0; i < RSA_NUM_WORDS; ++i)
		montMulAdd(key, c, a[i], b);
}

/* Montgomery c[] = a[] * 1 / R % key. */
void montMul1(const uint32_t *key,
		 uint32_t *c, const uint32_t *a)
{
	int i;

	for (i = 0; i < RSA_NUM_WORDS; ++i)
		c[i] = 0;

	montMulAdd(key, c, 1, a);
	for (i = 1; i < RSA_NUM_WORDS; ++i)
		montMulAdd(key, c, 0, a);
}


/* In-place exponentiation to power % key.
 * Pulled from Cr50 RO 0.0.14 
 */
void LOADERKEY_modpow(const uint32_t *key,
		const uint32_t *signature, uint32_t *out)
{
	static uint32_t aaR[RSA_NUM_WORDS];
	static uint32_t aaaR[RSA_NUM_WORDS];

	montMul(key, aaR, signature, signature);

	// Support for public exponent (2^16) + 1
	if (key[97] == 0x10001) {
		for (int i = 0; i < 7; ++i) {
			montMul(key, aaaR, aaR, aaR);
			montMul(key, aaR, aaaR, aaaR);
		}

		montMul(key, aaaR, aaR, aaR);
		memcpy(aaR, aaaR, RSA_NUM_BYTES);
	}

	montMul(key, aaaR, aaR, signature);
	montMul1(key, out, aaaR);
}

/* In place implementation of cbrt(msg invmod key) */
int LOADERKEY_invcube(const uint32_t *key,
		const uint32_t *message, uint32_t *out)
{
	/* Exponents higher than 3 can't be feasibly rooted like this. */
	if (key[97] > 3) {
		fprintf(stderr, "Error: Expected an exponent of 3, got %d!\n", key[97]);
		return -1;
	}

	/* TODO (Hannah): Implement message cube rooting. */
	return -1;
}
