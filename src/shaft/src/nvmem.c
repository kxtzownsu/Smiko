/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#include <openssl/sha.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip_config.h"
#include "header.h"

/* Cr50 NVMEM data */
#define CONFIG_CR50_NVMEM_A_OFF \
	(CONFIG_RW_SIZE)
#define CONFIG_CR50_NVMEM_B_OFF \
	(CFG_FLASH_HALF + CONFIG_RW_SIZE)
#define CONFIG_CR50_NVMEM_SIZE 0x3000

/* Ti50 filesystem data */
#define CONFIG_TI50_NVMEM_A_OFF \
	(CONFIG_DT_PROGRAM_MEMORY_BASE + CONFIG_DT_RW_SIZE)
#define CONFIG_TI50_NVMEM_B_OFF \
	(CFG_DT_FLASH_HALF + CONFIG_DT_RW_SIZE)
#define CONFIG_TI50_NVMEM_SIZE 0x7000


enum nvmem_users {
	NVMEM_TPM = 0,
	NVMEM_GSC,
	NVMEM_NUM_USERS
};