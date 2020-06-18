/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */


static u8 bus_read8(u16 addr);
static void bus_write8(u16 addr, u8 data);
static u16 bus_read16(u16 addr);
static void bus_write16(u16 addr, u16 data);

#ifndef BUS_H
#define BUS_H
// this file contains all memory logic to cpu

#include "defs.h"
#include "ppu.h"

// cpu does not have internal memory so it is connected to memory via bus
// 0x0 - 0xFFFF  adress range
// this allows to connect different devices to talk with the cpu and map them to this address range

// 0x0 - 0x1FFF cpu addressable range
#define CPU_MEMORY_START            0x0
#define CPU_MEMORY_SIZE             0x1FFF

// cpu memory is mirrored every 2 kilobytes
#define CPU_MEMORY_MIRROR_RANGE     0x07FF

#define PPU_MEMORY_START            0x2000
#define PPU_MEMORY_SIZE             0x3FFF

#define CONTROLLER1                 0x4016
#define CONTROLLER2                 0x4017

// 0x4020 - 0xFFFF cartridge range

// cpu memory
u8 ram[2048];
u8 buttonState[2];
u8 internalButtonState[2];

static u8
bus_read8(u16 addr) {

    u8 ret = 0;
    if(address_is_between(addr, CPU_MEMORY_START, CPU_MEMORY_SIZE)) {
        ret = ram[addr & CPU_MEMORY_MIRROR_RANGE];
    } else if(address_is_between(addr, PPU_MEMORY_START, PPU_MEMORY_SIZE)) {
        ret = ppu_cpu_read(addr);
    } else if (addr == CONTROLLER1) {
        ret = (buttonState[0] & 0x80) > 0;
        buttonState[0] <<= 1;
    } else if (addr == CONTROLLER2) {
        ret = (buttonState[1] & 0x80) > 0;
        buttonState[1] <<= 1;
    } else {
        ret = cartridge_cpu_read_rom(addr);
    }

    CHECKLOG;
#ifdef LOGFILE
    fprintf(logfile, "red 0x%04X from address 0x%04X\n", ret, addr);
#endif

    return ret;
}

static void
bus_write8(u16 addr, u8 data) {

    if(address_is_between(addr, CPU_MEMORY_START, CPU_MEMORY_SIZE)) {
        ram[addr & CPU_MEMORY_MIRROR_RANGE] = data;
    } else if(address_is_between(addr, PPU_MEMORY_START, PPU_MEMORY_SIZE)) {
        ppu_cpu_write(addr, data);
    } else if (addr == CONTROLLER1) {
        buttonState[0] = internalButtonState[0];
    } else if (addr == CONTROLLER2) {
        buttonState[1] = internalButtonState[1];
    } else {
        cartridge_cpu_write_rom(addr, data);
    }
    CHECKLOG;
#ifdef LOGFILE
    fprintf(logfile, "wrote 0x%04X to address 0x%04X\n", data, addr);
#endif
}

// ensure endianess check for this,
// 6502 is little endian
static u16
bus_read16(u16 addr) {

    u16 low = bus_read8(addr);
    u16 high = bus_read8(addr + 1);

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
