/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef MAPPERS_H
#define MAPPERS_H

// This file just implements the mapping functions

#define MAP0_START              0x8000
#define MAP0_END                0xFFFF
#define MAP0_PPU_DATA_SIZE      0x1FFF

u8
mapper0_cpu_peak(u16 addr) {

    if(!address_is_between(addr, MAP0_START, MAP0_END)) {
        return 0;
    }

    u16 ret = 0;
    if(cartridge.numProgramRoms == 1)
        ret = addr & 0x3FFF; // if 1 rom capasity is 16K
    else
        ret = addr & 0x7FFF; // if 2 rom capasity is 32K

    return ret;
}

u8
mapper0_cpu_read(u16 addr) {

    if(!address_is_between(addr, MAP0_START, MAP0_END)) {
        return 0;
    }

    if(cartridge.numProgramRoms == 1)
        addr &= 0x3FFF; // if 1 rom capasity is 16K
    else
        addr &= 0x7FFF; // if 2 rom capasity is 32K

    return cartridge.programMemory[addr];
}

void
mapper0_cpu_write(u16 addr, u8 val) {

    if(!address_is_between(addr, MAP0_START, MAP0_END)) {
        ABORT("invalid address in mapper0 0x%04X", addr);
        return;
    }

    if(cartridge.numProgramRoms == 1)
        addr &= 0x3FFF; // if 1 rom capasity is 16K
    else
        addr &= 0x7FFF; // if 2 rom capasity is 32K

    cartridge.programMemory[addr] = val;
}

u8
mapper0_ppu_read(u16 addr) {

    if(!address_is_between(addr, 0, MAP0_PPU_DATA_SIZE))
        ABORT("invalid address in mapper0 0x%04X", addr);

    return cartridge.characterMemory[addr];
}

void
mapper0_ppu_write(u16 addr, u8 val) {

    if(!address_is_between(addr, 0, MAP0_PPU_DATA_SIZE)) ABORT("invalid address in mapper0");

    cartridge.characterMemory[addr] = val;
}


struct Mapper mapper0 = {
    .cpu_peak_cartridge =   mapper0_cpu_peak,
    .cpu_read_cartridge =   mapper0_cpu_read,
    .cpu_write_cartridge =  mapper0_cpu_write,
    .ppu_read_cartridge =   mapper0_ppu_read,
    .ppu_write_cartridge =  mapper0_ppu_write,
    .programMemStart = MAP0_START
};

#define MAPPER1_CONTROL_BIT 0x8000

u8
mapper1_cpu_peak(u16 addr) {
    LOG("TODO");
    return 0;
}

u8
mapper1_cpu_read(u16 addr) {

    if(!address_is_between(addr, MAP0_START, MAP0_END)) {

        ABORT("invalid address in mapper0 0x%04X", addr);
        return 0;
    }

    u16 ret = 0;
    if(cartridge.numProgramRoms == 1)
        ret = addr & 0x3FFF; // if 1 rom capasity is 16K
    else
        ret = addr & 0x7FFF; // if 2 rom capasity is 32K

    return ret;
}

void
mapper1_cpu_write(u16 addr, u8 data) {

    if(!address_is_between(addr, MAP0_START, MAP0_END)) {
        ABORT("invalid address in mapper0 0x%04X", addr);
        return;
    }

    u16 ret = 0;
    if(cartridge.numProgramRoms == 1)
        ret = addr & 0x3FFF; // if 1 rom capasity is 16K
    else
        ret = addr & 0x7FFF; // if 2 rom capasity is 32K
}

u8
mapper1_ppu_read(u16 addr) {

    if(!address_is_between(addr, 0, MAP0_PPU_DATA_SIZE))
        ABORT("invalid address in mapper0 0x%04X", addr);

    return addr;
}

void
mapper1_ppu_write(u16 addr, u8 data) { // TODO??

    if(!address_is_between(addr, 0, MAP0_PPU_DATA_SIZE)) ABORT("invalid address in mapper0");

}


struct Mapper mapper1 = {
    .cpu_peak_cartridge = mapper1_cpu_peak,
    .cpu_read_cartridge = mapper1_cpu_read,
    .cpu_write_cartridge = mapper1_cpu_write,
    .ppu_read_cartridge = mapper1_ppu_read,
    .ppu_write_cartridge = mapper1_ppu_write,
    .programMemStart = MAP0_START
};

#endif /* MAPPERS_H */
