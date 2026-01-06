#include <stddef.h>
#include <stdint.h>

#include "vendor-code-string.h"
#include "tpm_vendor_cmds.h"

char *GetVendorCodeString(uint16_t command_code)
{
    char *str = NULL;

    switch(command_code) {
        case VENDOR_CC_GET_LOCK:
            str = "VENDOR_CC_GET_LOCK";
            break;
        case VENDOR_CC_SET_LOCK:
            str = "VENDOR_CC_SET_LOCK";
            break;
        case VENDOR_CC_SYSINFO:
            str = "VENDOR_CC_SYSINFO";
            break;
        case VENDOR_CC_IMMEDIATE_RESET:
            str = "VENDOR_CC_IMMEDIATE_RESET";
            break;
        case VENDOR_CC_INVALIDATE_INACTIVE_RW:
            str = "VENDOR_CC_INVALIDATE_INACTIVE_RW";
            break;
        case VENDOR_CC_COMMIT_NVMEM:
            str = "VENDOR_CC_COMMIT_NVMEM";
            break;
        case VENDOR_CC_REPORT_TPM_STATE:
            str = "VENDOR_CC_REPORT_TPM_STATE";
            break;
        case VENDOR_CC_TURN_UPDATE_ON:
            str = "VENDOR_CC_TURN_UPDATE_ON";
            break;
        case VENDOR_CC_GET_BOARD_ID:
            str = "VENDOR_CC_GET_BOARD_ID";
            break;
        case VENDOR_CC_SET_BOARD_ID:
            str = "VENDOR_CC_SET_BOARD_ID";
            break;
        case VENDOR_CC_U2F_APDU:
            str = "VENDOR_CC_U2F_APDU";
            break;
        case VENDOR_CC_POP_LOG_ENTRY:
            str = "VENDOR_CC_POP_LOG_ENTRY";
            break;
        case VENDOR_CC_GET_REC_BTN:
            str = "VENDOR_CC_GET_REC_BTN";
            break;
        case VENDOR_CC_RMA_CHALLENGE_RESPONSE:
            str = "VENDOR_CC_RMA_CHALLENGE_RESPONSE";
            break;
        case VENDOR_CC_DISABLE_FACTORY:
            str = "VENDOR_CC_DISABLE_FACTORY";
            break;
        case VENDOR_CC_CCD:
            str = "VENDOR_CC_CCD";
            break;
        case VENDOR_CC_GET_ALERTS_DATA:
            str = "VENDOR_CC_GET_ALERTS_DATA";
            break;
        case VENDOR_CC_SPI_HASH:
            str = "VENDOR_CC_SPI_HASH";
            break;
        case VENDOR_CC_PINWEAVER:
            str = "VENDOR_CC_PINWEAVER";
            break;
        case VENDOR_CC_RESET_FACTORY:
            str = "VENDOR_CC_RESET_FACTORY";
            break;
        case VENDOR_CC_WP:
            str = "VENDOR_CC_WP";
            break;
        case VENDOR_CC_TPM_MODE:
            str = "VENDOR_CC_TPM_MODE";
            break;
        case VENDOR_CC_SN_SET_HASH:
            str = "VENDOR_CC_SN_SET_HASH";
            break;
        case VENDOR_CC_SN_INC_RMA:
            str = "VENDOR_CC_SN_INC_RMA";
            break;
        case VENDOR_CC_GET_PWR_BTN:
            str = "VENDOR_CC_GET_PWR_BTN";
            break;
        case VENDOR_CC_U2F_GENERATE:
            str = "VENDOR_CC_U2F_GENERATE";
            break;
        case VENDOR_CC_U2F_SIGN:
            str = "VENDOR_CC_U2F_SIGN";
            break;
        case VENDOR_CC_U2F_ATTEST:
            str = "VENDOR_CC_U2F_ATTEST";
            break;
        case VENDOR_CC_FLOG_TIMESTAMP:
            str = "VENDOR_CC_FLOG_TIMESTAMP";
            break;
        case VENDOR_CC_ENDORSEMENT_SEED:
            str = "VENDOR_CC_ENDORSEMENT_SEED";
            break;
        case VENDOR_CC_U2F_MODE:
            str = "VENDOR_CC_U2F_MODE";
            break;
        case VENDOR_CC_DRBG_TEST:
            str = "VENDOR_CC_DRBG_TEST";
            break;
        case VENDOR_CC_TRNG_TEST:
            str = "VENDOR_CC_TRNG_TEST";
            break;
        case VENDOR_CC_GET_BOOT_MODE:
            str = "VENDOR_CC_GET_BOOT_MODE";
            break;
        case VENDOR_CC_RESET_EC:
            str = "VENDOR_CC_RESET_EC";
            break;
        case VENDOR_CC_SEED_AP_RO_CHECK:
            str = "VENDOR_CC_SEED_AP_RO_CHECK";
            break;
        case VENDOR_CC_FIPS_CMD:
            str = "VENDOR_CC_FIPS_CMD";
            break;
        case VENDOR_CC_GET_AP_RO_HASH:
            str = "VENDOR_CC_GET_AP_RO_HASH";
            break;
        case VENDOR_CC_GET_AP_RO_STATUS:
            str = "VENDOR_CC_GET_AP_RO_STATUS";
            break;
        case VENDOR_CC_AP_RO_VALIDATE:
            str = "VENDOR_CC_AP_RO_VALIDATE";
            break;
        case VENDOR_CC_DS_DIS_TEMP:
            str = "VENDOR_CC_DS_DIS_TEMP";
            break;
        case VENDOR_CC_USER_PRES:
            str = "VENDOR_CC_USER_PRES";
            break;
        case VENDOR_CC_POP_LOG_ENTRY_MS:
            str = "VENDOR_CC_POP_LOG_ENTRY_MS";
            break;
        case VENDOR_CC_GET_AP_RO_VERIFY_SETTING:
            str = "VENDOR_CC_GET_AP_RO_VERIFY_SETTING";
            break;
        case VENDOR_CC_SET_AP_RO_VERIFY_SETTING:
            str = "VENDOR_CC_SET_AP_RO_VERIFY_SETTING";
            break;
        case VENDOR_CC_SET_CAPABILITY:
            str = "VENDOR_CC_SET_CAPABILITY";
            break;
        case VENDOR_CC_GET_TI50_STATS:
            str = "VENDOR_CC_GET_TI50_STATS";
            break;
        case VENDOR_CC_GET_CRASHLOG:
            str = "VENDOR_CC_GET_CRASHLOG";
            break;
        case VENDOR_CC_GET_CONSOLE_LOGS:
            str = "VENDOR_CC_GET_CONSOLE_LOGS";
            break;
        case VENDOR_CC_GET_FACTORY_CONFIG:
            str = "VENDOR_CC_GET_FACTORY_CONFIG";
            break;
        case VENDOR_CC_SET_FACTORY_CONFIG:
            str = "VENDOR_CC_SET_FACTORY_CONFIG";
            break;
        case VENDOR_CC_GET_TIME:
            str = "VENDOR_CC_GET_TIME";
            break;
        case VENDOR_CC_GET_BOOT_TRACE:
            str = "VENDOR_CC_GET_BOOT_TRACE";
            break;
        case VENDOR_CC_GET_CHASSIS_OPEN:
            str = "VENDOR_CC_GET_CHASSIS_OPEN";
            break;
        case VENDOR_CC_GET_CR50_METRICS:
            str = "VENDOR_CC_GET_CR50_METRICS";
            break;
        case VENDOR_CC_GET_CHIP_ID:
            str = "VENDOR_CC_GET_CHIP_ID";
            break;
        default:
            str = "UNKNOWN";
            break;
    }

    return str;
}