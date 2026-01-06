/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SMIKO_CHIP_CONFIG_H
#define __SMIKO_CHIP_CONFIG_H

// TPM transfer info
#define SIGNED_TRANSFER_SIZE 1024
#define MAX_RX_BUF_SIZE 2048
#define MAX_TX_BUF_SIZE (SIGNED_TRANSFER_SIZE + sizeof(struct tpm_pkt))

// UART transfer info
#define UART_RX_BUF_SIZE 1024
#define UART_TX_BUF_SIZE 1024

// H1 flash region info
#define CONFIG_PROGRAM_MEMORY_BASE 0x40000
#define CONFIG_FLASH_SIZE (512 * 1024)
#define CFG_FLASH_HALF (CONFIG_FLASH_SIZE >> 1)
#define CONFIG_RO_SIZE 0x4000
#define CONFIG_RW_SIZE (CFG_FLASH_HALF - 0x4000 - 0x3000)

// D2 flash region info
#define CONFIG_DT_PROGRAM_MEMORY_BASE 0x80000
#define CONFIG_DT_FLASH_SIZE (1024 * 1024)
#define CFG_DT_FLASH_HALF (CONFIG_DT_FLASH_SIZE >> 1)
#define CONFIG_DT_RW_SIZE 0x77000
#endif /* __SMIKO_CHIP_CONFIG_H */