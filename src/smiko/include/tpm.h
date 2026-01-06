/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SMIKO_INCLUDE_TPM_H
#define __SMIKO_INCLUDE_TPM_H

#ifdef __cplusplus
extern "C" {
#endif

extern int gsc_socket;

/* Send GSC commands over TPM. */
int tpm_send_payload(unsigned int digest, unsigned int addr, 
                 const void *data, int size, uint16_t subcmd);

/* Read GSC command responses over TPM. */
int tpm_read_response(void *response, size_t *response_size);

#ifdef __cplusplus
}
#endif

#endif /* __SMIKO_INCLUDE_TPM_H */