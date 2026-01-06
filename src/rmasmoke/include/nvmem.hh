#ifndef __RMASMOKE_NVMEM_H
#define __RMASMOKE_NVMEM_H

#include <tss2/tss2_common.h>
#include <tss2/tss2_sys.h>
#include <tss2/tss2_tpm2_types.h>

#include "tss_context.h"

// TPM packet constructors.
TSS2_SYS_CONTEXT *GenerateContext(size_t *size);
struct tpm_result *GetCommandBufferFromSys(TSS2_SYS_CONTEXT *ctx);
void ClearTPMContext(TSS2_SYS_CONTEXT *ctx);
void PreComplete(TSS2_SYS_CONTEXT *sysctx);

// NV memory functions
int tpm_startup(void);

int nvmem_define(uint32_t index, TPM2B_NV_PUBLIC *pub);

int nvmem_undefine(uint32_t index);

// Function to write to an NV space.
void *nvmem_write(uint32_t index, uint32_t offset,
                  void *data, size_t len);

void *nvmem_read(uint32_t index, uint16_t offset, 
                 uint16_t length);

int nvmem_clear(void);

int nvmem_set_hierarchy(std::string owner_password,
                        std::string endorsement_password,
					    std::string lockout_password);

#endif /* __RMASMOKE_NVMEM_H */