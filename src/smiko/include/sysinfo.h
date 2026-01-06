/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SMIKO_INCLUDE_INFO_H
#define __SMIKO_INCLUDE_INFO_H

#include "tpm_vendor_cmds.h"

#ifdef __cplusplus
extern "C" {
#endif

int get_sysinfo(struct first_response_pdu rpdu);

int get_headerinfo(const char *path);

int get_keyinfo(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* __SMIKO_INCLUDE_INFO_H */