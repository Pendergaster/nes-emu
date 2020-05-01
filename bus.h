/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef BUS_H
#define BUS_H

#include "defs.h"

// cpu does not have internal memory so it is connected to memory via bus
// 0x0 - 0xFFFF  adress range
// this allows to connect different devices to talk with the cpu and map them to this address range


u8 ram[0xFFFF];

static u8
bus_read(u16 addr) {
    return ram[addr];
}

static void
bus_write(u16 addr, u8 data) {

    ram[addr] = data;
}

#endif /* BUS_H */
