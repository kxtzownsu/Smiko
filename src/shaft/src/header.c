/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "args.h"
#include "chip_config.h"
#include "signed_header.h"

bool valid_signed_header(const struct SignedHeader *header)
{
	if (header->magic != MAGIC_HAVEN && 
		header->magic != MAGIC_CITADEL &&
		header->magic != MAGIC_DAUNTLESS)
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