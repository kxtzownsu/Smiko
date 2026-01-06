/* Copyright 2015 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __CROS_EC_TPM_VENDOR_CMDS_H
#define __CROS_EC_TPM_VENDOR_CMDS_H

#include "signed_header.h"

#ifndef BIT
#define BIT(nr)         (1UL << (nr))
#endif

enum vendor_cmd_cc {
	/* Original extension commands */
	EXTENSION_AES = 0,
	EXTENSION_HASH = 1,
	EXTENSION_RSA = 2,
	EXTENSION_ECC = 3,
	EXTENSION_FW_UPGRADE = 4,
	EXTENSION_HKDF = 5,
	EXTENSION_ECIES = 6,
	EXTENSION_POST_RESET = 7,

	LAST_EXTENSION_COMMAND = 15,

	/* Our TPMv2 vendor-specific command codes. 16 bits available. */
	VENDOR_CC_GET_LOCK = 16,
	VENDOR_CC_SET_LOCK = 17,
	VENDOR_CC_SYSINFO = 18,
	/*
	 * VENDOR_CC_IMMEDIATE_RESET may have an argument, which is a (uint16_t)
	 * time delay (in milliseconds) in doing a reset. Max value is 1000.
	 * The command may also be called without an argument, which will be
	 * regarded as zero time delay.
	 */
	VENDOR_CC_IMMEDIATE_RESET = 19,
	VENDOR_CC_INVALIDATE_INACTIVE_RW = 20,
	VENDOR_CC_COMMIT_NVMEM = 21,
	/* DEPRECATED(22): deep sleep control command. */
	VENDOR_CC_REPORT_TPM_STATE = 23,
	VENDOR_CC_TURN_UPDATE_ON = 24,
	VENDOR_CC_GET_BOARD_ID = 25,
	VENDOR_CC_SET_BOARD_ID = 26,
	VENDOR_CC_U2F_APDU = 27,
	VENDOR_CC_POP_LOG_ENTRY = 28,
	VENDOR_CC_GET_REC_BTN = 29,
	VENDOR_CC_RMA_CHALLENGE_RESPONSE = 30,
	/* DEPRECATED(31): CCD password command (now part of VENDOR_CC_CCD) */
	/*
	 * Disable factory mode. Reset all ccd capabilities to default and reset
	 * write protect to follow battery presence.
	 */
	VENDOR_CC_DISABLE_FACTORY = 32,
	/* DEPRECATED(33): Manage CCD password phase */
	VENDOR_CC_CCD = 34,
	VENDOR_CC_GET_ALERTS_DATA = 35,
	VENDOR_CC_SPI_HASH = 36,
	VENDOR_CC_PINWEAVER = 37,
	/*
	 * Check the factory reset settings. If they're all set correctly, do a
	 * factory reset to enable ccd factory mode. All capabilities will be
	 * set to Always and write protect will be permanently disabled. This
	 * mode can't be reset unless VENDOR_CC_DISABLE_FACTORY is called or
	 * the 'ccd reset' console command is run.
	 */
	VENDOR_CC_RESET_FACTORY = 38,
	/*
	 * Get the write protect setting. This will return a single byte with
	 * bits communicating the write protect setting as described by the
	 * WPV subcommands.
	 */
	VENDOR_CC_WP = 39,
	/*
	 * Either enable or disable TPM mode. This is allowed for one-time only
	 * until next TPM reset EVENT. In other words, once TPM mode is set,
	 * then it cannot be altered to the other mode value. The allowed input
	 * values are either TPM_MODE_ENABLED or TPM_MODE_DISABLED as defined
	 * in 'enum tpm_modes', tpm_registers.h.
	 * If the input size is zero, it won't change TPM_MODE.
	 * If either the input size is zero or the input value is valid,
	 * it will respond with the current tpm_mode value in uint8_t format.
	 *
	 *  Return code:
	 *   VENDOR_RC_SUCCESS: completed successfully.
	 *   VENDOR_RC_INTERNAL_ERROR: failed for an internal reason.
	 *   VENDOR_RC_NOT_ALLOWED: failed in changing TPM_MODE,
	 *                          since it is already set.
	 *   VENDOR_RC_NO_SUCH_SUBCOMMAND: failed because the given input
	 *                                 is undefined.
	 */
	VENDOR_CC_TPM_MODE = 40,
	/*
	 * Initializes INFO1 SN data space, and sets SN hash. Takes three
	 * int32 as parameters, which are written as the SN hash.
	 */
	VENDOR_CC_SN_SET_HASH = 41,
	/*
	 * Increments the RMA count in the INFO1 SN data space. The space must
	 * have been previously initialized with the _SET_HASH command above for
	 * this to succeed. Takes one byte as parameter, which indicates the
	 * number to increment the RMA count by; this is typically 1 or 0.
	 *
	 * Incrementing the RMA count by 0 will set the RMA indicator, but not
	 * incremement the count. This is useful to mark that a device has been
	 * RMA'd, but that we were not able to log the new serial number.
	 *
	 * Incrementing the count by the maximum RMA count (currently 7) will
	 * always set the RMA count to the maximum value, regardless of the
	 * previous value. This can be used with any device, regardless of
	 * current state, to mark it as RMA'd but with an unknown RMA count.
	 */
	VENDOR_CC_SN_INC_RMA = 42,

	/*
	 * Gets the latched state of a power button press to indicate user
	 * recent user presence. The power button state is automatically cleared
	 * after PRESENCE_TIMEOUT.
	 */
	VENDOR_CC_GET_PWR_BTN = 43,

	/*
	 * U2F commands.
	 */
	VENDOR_CC_U2F_GENERATE = 44,
	VENDOR_CC_U2F_SIGN = 45,
	VENDOR_CC_U2F_ATTEST = 46,

	VENDOR_CC_FLOG_TIMESTAMP = 47,
	VENDOR_CC_ENDORSEMENT_SEED = 48,

	VENDOR_CC_U2F_MODE = 49,

	/*
	 * HMAC-SHA256 DRBG invocation for ACVP tests
	 */
	VENDOR_CC_DRBG_TEST = 50,

	VENDOR_CC_TRNG_TEST = 51,

	/* EC EFS(Early Firmware Selection) commands */
	VENDOR_CC_GET_BOOT_MODE = 52,
	VENDOR_CC_RESET_EC = 53,

	VENDOR_CC_SEED_AP_RO_CHECK = 54,

	VENDOR_CC_FIPS_CMD = 55,

	VENDOR_CC_GET_AP_RO_HASH = 56,

	VENDOR_CC_GET_AP_RO_STATUS = 57,

	VENDOR_CC_AP_RO_VALIDATE = 58,

	/*
	 * Vendor command to disable deep sleep during the next TPM_RST_L
	 * assertion. Cr50 used to use 22 to do this. It can't reuse that
	 * because some old boards still send it, and deep sleep shouldn't
	 * be disabled on those boards.
	 */
	VENDOR_CC_DS_DIS_TEMP = 59,

	VENDOR_CC_USER_PRES = 60,

	/* POP_LOG_ENTRY with a 64 bit previous timestamp in ms */
	VENDOR_CC_POP_LOG_ENTRY_MS = 61,

	/*
	 * Get/set AP RO configuration settings
	 *
	 * The message sent and received to this vendor command,
	 * with the exception * of SET responses, uses the
	 * following form:
	 *
	 * ```c
	 * struct __attribute__((__packed__)) command_msg {
	 *   // Current version of the API
	 *   uint8_t version;
	 *   // Determines payload type, see
	 *   // `arv_config_setting_command_e`.
	 *   uint8_t command;
	 *   // Type here depends on command
	 *   struct command_data data;
	 * };
	 * ```
	 */
	VENDOR_CC_GET_AP_RO_VERIFY_SETTING = 62,
	VENDOR_CC_SET_AP_RO_VERIFY_SETTING = 63,

	/* Ti50 only. */
	VENDOR_CC_SET_CAPABILITY = 64,
	VENDOR_CC_GET_TI50_STATS = 65,
	VENDOR_CC_GET_CRASHLOG = 66,
	VENDOR_CC_GET_CONSOLE_LOGS = 67,

	VENDOR_CC_GET_FACTORY_CONFIG = 68,
	VENDOR_CC_SET_FACTORY_CONFIG = 69,

	VENDOR_CC_GET_TIME = 70,

	VENDOR_CC_GET_BOOT_TRACE = 71,

	VENDOR_CC_GET_CHASSIS_OPEN = 72,
	/*
	 * 72 was also the old VENDOR_CC_GET_CR50_METRICS value. It was moved
	 * to avoid conflict with ti50.
	 */
	VENDOR_CC_GET_CR50_METRICS = 73,
	
	/*
	 * Used for UMA collection for feature launch. After feature launch,
	 * this can be removed as long as the value is reserved.
	 * Cr50 doesn.t use this.
	 * VENDOR_CC_GET_AP_RO_RESET_COUNTS = 74,
	 */
	/* Returns info to identify the specific GSC chip type. */
	VENDOR_CC_GET_CHIP_ID = 75,

	LAST_VENDOR_COMMAND = 65535,
};

#define CONFIG_EXTENSION_COMMAND 0xbaccd00a
#define TPM_CC_VENDOR_BIT_MASK 0x20000000

enum vendor_cmd_rc {
	/* EXTENSION_HASH error codes */
	/* Attempt to start a session on an active handle. */
	EXC_HASH_DUPLICATED_HANDLE = 1,
	EXC_HASH_TOO_MANY_HANDLES = 2, /* No room to allocate a new context. */
	/* Continuation/finish on unknown context. */
	EXC_HASH_UNKNOWN_CONTEXT = 3,

	/* Our TPMv2 vendor-specific response codes. */
	VENDOR_RC_SUCCESS = 0,
	VENDOR_RC_BOGUS_ARGS = 1,
	VENDOR_RC_READ_FLASH_FAIL = 2,
	VENDOR_RC_WRITE_FLASH_FAIL = 3,
	VENDOR_RC_REQUEST_TOO_BIG = 4,
	VENDOR_RC_RESPONSE_TOO_BIG = 5,
	VENDOR_RC_INTERNAL_ERROR = 6,
	VENDOR_RC_NOT_ALLOWED = 7,
	VENDOR_RC_NO_SUCH_SUBCOMMAND = 8,
	VENDOR_RC_IN_PROGRESS = 9,
	VENDOR_RC_PASSWORD_REQUIRED = 10,
	VENDOR_RC_NVMEM_LOCKED = 11,

	/* Maximum possible failure reason. */
	VENDOR_RC_NO_SUCH_COMMAND = 127,

	/*
	 * Bits 10 and 8 set, this is to be ORed with the rest of the error
	 * values to make the combined value compliant with the spec
	 * requirements.
	 */
	VENDOR_RC_ERR = 0x500,
};

enum tpm_modes {
	TPM_MODE_ENABLED_TENTATIVE = 0,
	TPM_MODE_ENABLED = 1,
	TPM_MODE_DISABLED = 2,
	TPM_MODE_MAX,
};

/* Various upgrade command return values. */
enum return_value {
	UPGRADE_SUCCESS = 0,
	UPGRADE_BAD_ADDR = 1,
	UPGRADE_ERASE_FAILURE = 2,
	UPGRADE_DATA_ERROR = 3,
	UPGRADE_WRITE_FAILURE = 4,
	UPGRADE_VERIFY_ERROR = 5,
	UPGRADE_GEN_ERROR = 6,
	UPGRADE_MALLOC_ERROR = 7,
	UPGRADE_ROLLBACK_ERROR = 8,
	UPGRADE_RATE_LIMIT_ERROR = 9,
	UPGRADE_UNALIGNED_BLOCK_ERROR = 10,
	UPGRADE_TRUNCATED_HEADER_ERROR = 11,
	UPGRADE_BOARD_ID_ERROR = 12,
	UPGRADE_BOARD_FLAGS_ERROR = 13,
	UPGRADE_DEV_ID_MISMATCH_ERROR = 14,
};

/* Returns info to identify the specific GSC chip type. */
struct get_chip_id_response {
	uint32_t tpm_did_vid;
	uint32_t chip_id;
};

/*
 * Type of the GSC device. This is used to represent which type of GSC we are
 * connected to and to tag an image file for compatibility.
 * for downloading.
 */
enum gsc_device {
	GSC_DEVICE_H1,
	GSC_DEVICE_DT,
	GSC_DEVICE_NT,
};

/*
 * VENDOR_CC_WP options, only WP_ENABLE is accepted for cr50. For ti50,
 * enable, disable, and follow are all supported.
 */
enum wp_options {
	WP_NONE,
	WP_CHECK,
	WP_ENABLE,
	WP_DISABLE,
	WP_FOLLOW,
};

/*
 * Subcommand code, used to set write protect.
 */
#define WPV_UPDATE	     BIT(0)
#define WPV_ENABLE	     BIT(1)
#define WPV_FORCE	     BIT(2)
#define WPV_ATBOOT_SET	     BIT(3)
#define WPV_ATBOOT_ENABLE    BIT(4)
#define WPV_FWMP_FORCE_WP_EN BIT(5)

// Packet to be recieved during connection establishment.
struct first_response_pdu {
	uint32_t return_value;

	/* The below fields are present in versions 2 and up. */
	uint32_t protocol_version;

	/* The below fields are present in versions 3 and up. */
	uint32_t backup_ro_offset;
	uint32_t backup_rw_offset;

	/* The below fields are present in versions 4 and up. */
	/* Versions of the currently active RO and RW sections. */
	struct signed_header_version shv[2];

	/* The below fields are present in versions 5 and up */
	/* keyids of the currently active RO and RW sections. */
	uint32_t keyid[2];
};

// Response from VENDOR_CC_SYSINFO
struct sys_info_repsonse {
	uint32_t ro_keyid;
	uint32_t rw_keyid;
	uint32_t dev_id0;
	uint32_t dev_id1;
};

#endif /* __CROS_EC_TPM_VENDOR_CMDS_H */