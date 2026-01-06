/* The following is a watered down implementation of IHEX
 * conversion from cr50-codesigner. This should come in handy with
 * Carver and Smiko so we can handle IHEX images properly
 * (since for some reason Google loves storing their images in it) 
 */

#ifndef __SMIKO_IHEX_H
#define __SMIKO_IHEX_H

void binToIntelHex(FILE* fout, const uint8_t *mem_, 
                   uint32_t base_, uint32_t high_)
{
	for (int i = base_; i < high_; i += 16) {
		// spit out segment record at start of segment.
		if (!((i - base_) & 0xffff)) {
			int s = 0x4000 + (base_ >> 4) + ((i - base_) >> 4);
			fprintf(fout, ":02000002%04X%02X\n", s,
				(~((2 + 2 + (s >> 8)) & 255) + 1) & 255);
		}

		// spit out data records, 16 bytes each.
		fprintf(fout, ":10%04X00", (i - base_) & 0xffff);
		int crc = 16 + (((i - base_) >> 8) & 255) + ((i - base_) & 255);
		for (int n = 0; n < 16; ++n) {
			fprintf(fout, "%02X", mem_[i + n]);
			crc += mem_[i + n];
		}
		
		fprintf(fout, "%02X", (~(crc & 255) + 1) & 255);
		fprintf(fout, "\n");
	}
}

#endif /* __SMIKO_IHEX_H */