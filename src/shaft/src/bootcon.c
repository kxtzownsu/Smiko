#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "transfer.h"

/* Use this to write a string over Shaft's transfer convention. */
#define transfer_string(val) \
    transfer_write(val, strlen(val)); \
    usleep(100000)


void send_bootcon(void)
{
    transfer_init();

    transfer_string("rw 0x40090140 0x10000\n");      // GREG32(GLOBALSEC, CPU0_D_REGION2_BASE_ADDR) = 0x10000;
    transfer_string("rw 0x40090144 0x1f\n");         // GREG32(GLOBALSEC, CPU0_D_REGION2_SIZE) = SHA256_DIGEST_LENGTH - 1;
    transfer_string("rw 0x40090008 0x7\n");          // GREG32(GLOBALSEC, CPU0_D_REGION2_CTRL) = 0x7;
    transfer_string("rw 0x4009028c 0x40000\n");      // GREG32(GLOBALSEC, CPU0_I_STAGING_REGION2_BASE_ADDR) = CONFIG_PROGRAM_MEMORY_BASE;
    transfer_string("rw 0x40090290 0x80000\n");      // GREG32(GLOBALSEC, CPU0_I_STAGING_REGION2_SIZE) = CONFIG_FLASH_SIZE;
    transfer_string("rw 0x40090288 0x3\n");          // GREG32(GLOBALSEC, CPU0_I_STAGING_REGION2_CTRL) = 0x3;
    transfer_string("rw 0x40091004 0xe303ec7a\n");   // GREG32(GLOBALSEC, SB_BL_SIG0) = 0xe303ec7a;
    transfer_string("rw 0x40091008 0x68a03a27\n");   // GREG32(GLOBALSEC, SB_BL_SIG1) = 0x68a03a27;
    transfer_string("rw 0x4009100c 0xdd18053e\n");   // GREG32(GLOBALSEC, SB_BL_SIG2) = 0xdd18053e;
    transfer_string("rw 0x40091010 0x39f8dbbd\n");   // GREG32(GLOBALSEC, SB_BL_SIG3) = 0x39f8dbbd;
    transfer_string("rw 0x40091014 0x9b553578\n");   // GREG32(GLOBALSEC, SB_BL_SIG4) = 0x9b553578;
    transfer_string("rw 0x40091018 0xb4598244\n");   // GREG32(GLOBALSEC, SB_BL_SIG5) = 0xb4598244;
    transfer_string("rw 0x4009101c 0xc59f62d1\n");   // GREG32(GLOBALSEC, SB_BL_SIG6) = 0xc59f62d1;
    transfer_string("rw 0x40091020 0x61b8509e\n");   // GREG32(GLOBALSEC, SB_BL_SIG7) = 0x61b8509e;
    transfer_string("rw 0x40091024 0x0\n");          // GREG32(GLOBALSEC, SIG_UNLOCK) = 0x0;
    
    transfer_string("rw 0x44404"); // Read the entrypoint vector
    printf("BootCon has been set up, open the CCD console and run 'sysjump param', \n");
    printf("with 'param' replaced with the value the console says is at 0x44404.\n");

    transfer_close();
}
