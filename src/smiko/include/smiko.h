/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SMIKO_INCLUDE_SMIKO_H
#define __SMIKO_INCLUDE_SMIKO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum transfer_type {
    XFER_TPM_SOCKET,
    XFER_DT_SPI_DRIVER,
    XFER_CT_SPI_DRIVER,
    XFER_SUZYQ_USB,
};

extern enum transfer_type xfer_type;

/* Find a GSC interface. */
int open_socket(void);

uint8_t *read_file(const char *path, uint32_t *size_ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SMIKO_INCLUDE_SMIKO_H */