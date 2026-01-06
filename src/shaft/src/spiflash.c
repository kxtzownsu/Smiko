/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

// TODO (Hannah): There doesn't seem to be any spiflash-specific code needed, remove this file?

#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "args.h"
#include "chip_config.h"
#include "rescue.h"
#include "shaft.h"
#include "signed_header.h"

/*
 * Start SPIflash Bootstrap code. 
 *
 * SPIflash bootstrapping is a feature built into every GSC's BootROM.
 * This feature enables flashing of the A and B flash banks, and is designed
 * to serve as the bootstrap process for the chip during factory.
 *
 * This feature is done entirely over an SPI interface, and is what 
 * rescue was based off of. The two function very similarly, albeit
 * with a handful of key differences.
 *
 * Transfer starts as follows: The host prepares a firmware image, and resets
 * the target GSC. Before the BootROM launches an image, it checks if the dev_mode
 * line is high on the targer. If the line is pulled high, the ROM falls into BootStrap
 * mode, and accepts data into an RX buffer where it will then be programmed
 * into the requested address, so long as it falls between the A and B flash
 * banks. After the first packet validation and before flashing, both banks
 * are bulk-erased to allow a full reprogramming.
 * 
 * After transfer, the ROM continues with its verified boot process as normal,
 * RSA verifying the newly programmed images before executing them.
 */
int engage_bootstrap(char *tty)
{
    return 0;
}