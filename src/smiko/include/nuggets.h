#ifndef __SRC_SMIKO_INCLUDE_NUGGETS_H
#define __SRC_SMIKO_INCLUDE_NUGGETS_H

/* Send and recieve data over the Citadel/Dauntless driver. */
int nos_send_payload(unsigned int digest, unsigned int addr, 
					 const void *data, int size, uint16_t subcmd,
					 void *resp, size_t *resp_size);

#endif /* __SRC_SMIKO_INCLUDE_NUGGETS_H */