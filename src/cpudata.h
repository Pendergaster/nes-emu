/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef CPUDATA_H
#define CPUDATA_H

#include "defs.h"

#define STACK_START             0x0100
#define STACK_SIZE              0xFF
#define PROGRAM_START_POINTER   0xFFFC

// https://www.pagetable.com/?p=410
#define NMI_PC_LOCATION         0xFFFA
#define RESET_PC_LOCATION       0xFFFC
#define IRQ_OR_BRK_PC_LOCATION  0xFFFE

typedef struct cpu2ao3 {
    //  reqisters
    u8  Xreq;
    u8  Yreq;
    u8  accumReq;

    //  Processor status
    u8  flags;
    //  Program counter
    u16 pc;

    u8  stackPointer;
    u32 cycles;

    u64 instructionCount;
} cpu2ao3;

// global cpu variable
cpu2ao3 cpu = {
    .Xreq = 0, .Yreq = 0, .accumReq = 0, .flags = 0, .pc = 0x0, // pc is read from program start ptr
    .stackPointer = STACK_SIZE, .cycles = 0
};

#endif /* CPUDATA_H */
