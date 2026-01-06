/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#ifndef __SMIKO_INCLUDE_USB_H
#define __SMIKO_INCLUDE_USB_H

#ifdef __cplusplus
extern "C" {
#endif

// USB transfer info
#define H1_PID 0x5014
#define D2_PID 0x504A
#define NT_PID 0x5066
#define G_VID 0x18d1
#define USB_SUBCLASS_GOOGLE_CR50 0x53
#define USB_PROTOCOL_GOOGLE_CR50_NON_HC_FW_UPDATE 0xff

#define UPGRADE_DONE 0xB007AB1E /* USB notification the upgrade is finished sending. */

/* USB frames for vendor/extension commands */
struct upgrade_command {
	uint32_t block_digest;
	uint32_t block_base;
};
struct update_pdu {
	uint32_t block_size;
	struct upgrade_command cmd;
};

/* This describes USB endpoint used to communicate with Cr50. */
struct usb_endpoint {
	struct libusb_device_handle *devh;
	uint8_t ep_num;
	int chunk_len;
};

#define USB_ERROR(m, r)                                                        \
	fprintf(stderr, "Error: %s:%d, %s returned %d (%s)\n", __FILE__, __LINE__, m, r, libusb_strerror(r))


extern struct usb_endpoint uep;


void do_xfer(struct usb_endpoint *endpoint, void *outbuf, int outlen,
		    void *inbuf, int inlen, int allow_less, size_t *rxed_count);

void usb_shut_down(struct usb_endpoint *uep);

int usb_findit(const char *serial, uint16_t vid, uint16_t pid,
	       uint16_t subclass, uint16_t protocol, struct usb_endpoint *uep);

/* Send and recieve data over USB. */
int usb_send_payload(unsigned int digest, unsigned int addr, 
					 const void *data, int size, uint16_t subcmd,
					 void *resp, size_t *resp_size);

#ifdef __cplusplus
}
#endif

#endif /* __SMIKO_INCLUDE_USB_H */