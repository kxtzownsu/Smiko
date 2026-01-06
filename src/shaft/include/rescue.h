/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SHAFT_RESCUE_H
#define __SHAFT_RESCUE_H

// Write the UART packet to engage rescue mode
int engage_rescue(const char *tty);

void extort_rescue(const char *tty, int frame);

int bin2rec(const void *data, size_t len,
		uint32_t addr, char *output);

#endif /* __SHAFT_RESCUE_H */