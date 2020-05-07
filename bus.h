/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef BUS_H
#define BUS_H

#include "defs.h"

// cpu does not have internal memory so it is connected to memory via bus
// 0x0 - 0xFFFF  adress range
// this allows to connect different devices to talk with the cpu and map them to this address range


// 0x0 - 0x1FFF cpu addressable range
// 0x4020 - 0xFFFF cartridge range
u8 ram[0xFFFF];

static u8
bus_read8(u16 addr) {
    return ram[addr];
}

static void
bus_write8(u16 addr, u8 data) {

    ram[addr] = data;
}

// ensure endianess check for this,
// 6502 is little endian
static u16
bus_read16(u16 addr) {
    u16 low = ram[addr];
    u16 high = ram[addr + 1];
    return (high << 8) | low;
}

// ensure endianess check for this,
// 6502 is little endian
static void
bus_write16(u16 addr, u16 data) {
    u16 high = (data >> 8);
    u16 low = data & 0x00FF;

    bus_write8(addr, low);
    bus_write8(addr + 1, high);
}

#endif /* BUS_H */
