/* Copyright 2017 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stddef.h>
#include <stdint.h>

#include "signed_header.h"
#include "tpm_vendor_cmds.h"
#include "board_id.h"
#include "vendor_cmds.h"


int board_id_type_is_blank(const struct board_id *id)
{
	return (id->id & id->mask) == BLANK_FIELD;
}

int board_id_flags_are_blank(const struct board_id *id)
{
	return id->flags == BLANK_FIELD;
}

int board_id_is_blank(const struct board_id *id)
{
	return board_id_type_is_blank(id) && board_id_flags_are_blank(id);
}

uint32_t check_board_id_vs_header(const struct board_id *id,
				  const struct SignedHeader *h)
{
	uint32_t mismatch;
	uint32_t header_board_id_type;
	uint32_t header_board_id_mask;
	uint32_t header_board_id_flags;

	header_board_id_type = SIGNED_HEADER_PADDING ^ h->board_id.id;
	header_board_id_mask = SIGNED_HEADER_PADDING ^ h->board_id.mask;
	header_board_id_flags = SIGNED_HEADER_PADDING ^ h->board_id.flags;

	/*
	 * All 1-bits in header Board ID flags must be present in flags from
	 * flash
	 */
	mismatch =
		((header_board_id_flags & id->flags) != header_board_id_flags);
	/*
	 * Masked bits in header Board ID type must match type and inverse from
	 * flash.
	 */
	if (!mismatch && !board_id_type_is_blank(id)) {
		mismatch = header_board_id_type ^ id->id;
		mismatch |= header_board_id_type ^ ~id->mask;
		mismatch &= header_board_id_mask;
	}
	
	return mismatch;
}

int write_board_id(struct board_id *bid)
{
	return 0;
}