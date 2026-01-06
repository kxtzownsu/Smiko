/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SMIKO_INCLUDE_TRUNKS_H
#define __SMIKO_INCLUDE_TRUNKS_H

#ifdef __cplusplus
extern "C" {
#endif

/* File handle to share between write and read sides. */
int trunks_write(const void *out, size_t len);

/* Read incoming TPM data from trunks */
int trunks_read(void *buf, size_t max_rx_size);

#ifdef __cplusplus
}
#endif

#endif /* __SMIKO_INCLUDE_TRUNKS_H */