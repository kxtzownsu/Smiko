/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SHAFT_INCLUDE_HEADER_H
#define __SHAFT_INCLUDE_HEADER_H

#include "signed_header.h"

/* Rounds and address up to the next 2KB boundary if not one already */
uint32_t round_up_2kb(const uint32_t addr);

bool valid_signed_header(const struct SignedHeader *header);

bool valid_signed_manifest(const struct SignedManifest *manifest);

/* Returns the RW header or -1 if one cannot be found */
uint32_t find_rw_header(const void *data, size_t size, uint32_t offset);

#endif /* __SHAFT_INCLUDE_HEADER_H */