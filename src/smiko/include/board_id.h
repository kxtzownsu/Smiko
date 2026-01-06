/* Copyright 2017 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Modified by Hannah for use in Smiko outside of Cr50.
 */

#ifndef __SMIKO_BOARD_ID_H
#define __SMIKO_BOARD_ID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "signed_header.h"

struct board_id {
	uint32_t id;
	uint32_t mask;
	uint32_t flags;
};

#define BLANK_FIELD 0xffffffff

int board_id_type_is_blank(const struct board_id *id);

int board_id_flags_are_blank(const struct board_id *id);

int board_id_is_blank(const struct board_id *id);

uint32_t check_board_id_vs_header(const struct board_id *id,
				  const struct SignedHeader *h);

#ifdef __cplusplus
}
#endif

#endif /* __SMIKO_BOARD_ID_H */