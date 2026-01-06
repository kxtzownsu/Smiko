/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SRC_SHAFT_INCLUDE_TRANSFER_H
#define __SRC_SHAFT_INCLUDE_TRANSFER_H

struct bootstrap_pkt {
    uint8_t hash[32];
    uint32_t frame_num;
    uint32_t flash_offset;
    uint8_t data[0];
};

/**
 * Open a transfer descriptor on a provided driver.
 */
void transfer_init(void);

/**
 * Write a data packet to the GSC.
 *
 * @param data: Buffer containing data to transfer.
 * @param len: Size of data to TX.
 * 
 * @return Number of bytes TX'd.
 */
int transfer_write(void *data, size_t len);



/**
 * Read a transferred response from the GSC.
 *
 * @param out: Buffer to place read data at.
 * @param len: Size of data to RX.
 * 
 * @return Number of bytes RX'd.
 */
int transfer_read(void *out, size_t len);


/**
 * Synchronize the GSC rxbuf pointer with ours.
 *
 * @return 1 on success, 0 on failure.
 */
int transfer_syncronize();

void transfer_close(void);

/** 
 * Transfer a chunk of data over a provided channel to a connected target.
 * 
 * @param data:  Pointer to the chunk of data to be transferred.
 * @param size:  Size of the input data to be transferred.
 * @param adr:   Offset in the target's flash memory for the sent data
 *               to be stored.
 * @param frame_start: Starting frame number. Usually should be 0.
 * 
 * @return The frame number transfer ended on.
 */
int transfer_chunk(const uint8_t *data, size_t size, 
                   uint32_t adr, int frame_start);


/**
 * End transfer on the provided driver.
 *
 * @return 0 on Success.
 */
int transfer_end(void);

#endif /* __SRC_SHAFT_INCLUDE_TRANSFER_H */