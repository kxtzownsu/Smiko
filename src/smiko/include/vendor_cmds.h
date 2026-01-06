/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SMIKO_INCLUDE_VENDOR_CMDS_H
#define __SMIKO_INCLUDE_VENDOR_CMDS_H

#ifdef __cplusplus
extern "C" {
#endif

union gsc_start_resp {
	struct first_response_pdu rpdu;
	uint32_t legacy_resp;
};

int get_board_id(struct board_id *bid);

int get_cr50_info(struct sys_info_repsonse *info);

int get_wp_state(uint8_t *ret);

int get_boot_mode(uint8_t *ret);

int get_console_lock(uint8_t *ret);

int get_cr50_metrics(struct cr50_stats_response *resp);

enum gsc_device get_chip_id(void);

int transfer_chunk(uint8_t *data_ptr, uint32_t addr, size_t data_len);

int write_rom(const char *path);

int write_stock(const char *path);

int write_firmware(const char *path);

int reset_gsc(void);

int connect_to_target(void);

#ifdef __cplusplus
}
#endif

#endif /* __SMIKO_INCLUDE_VENDOR_CMDS_H */