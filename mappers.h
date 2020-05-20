/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef MAPPERS_H
#define MAPPERS_H

// This file just implements the mapping functions

#define MAP0_START              0x8000
#define MAP0_END                0xFFFF
#define MAP0_PPU_DATA_SIZE      0x1FFF

u16
mapper0_cpu_read(u16 addr) {

    if(!address_is_between(addr, MAP0_START, MAP0_END)) {
        return 0;
        //ABORT("invalid address in mapper0 0x%04X", addr);
    }

    u16 ret = 0;
    if(cartridge.numProgramRoms == 1)
        ret = addr & 0x3FFF; // if 1 rom capasity is 16K
    else
        ret = addr & 0x7FFF; // if 2 rom capasity is 32K

    return ret;
}

u16
mapper0_cpu_write(u16 addr) {

    if(!address_is_between(addr, MAP0_START, MAP0_END))
        return 0;
        //ABORT("invalid address in mapper0 0x%04X", addr);

    u16 ret = 0;
    if(cartridge.numProgramRoms == 1)
        ret = addr & 0x3FFF; // if 1 rom capasity is 16K
    else
        ret = addr & 0x7FFF; // if 2 rom capasity is 32K

    return ret;
}

u16
mapper0_ppu_read(u16 addr) {

    if(!address_is_between(addr, 0, MAP0_PPU_DATA_SIZE))
        ABORT("invalid address in mapper0 0x%04X", addr);

    return addr;
}

u16
mapper0_ppu_write(u16 addr) { // TODO??

    (void) addr;
    ABORT("mapper0 cant write to ppu");
    //if(!address_is_between(addr, 0, MAP0_PPU_DATA_SIZE)) ABORT("invalid address in mapper0");

    return addr;
}


struct Mapper mapper0 = {
    .cpu_translate_read = mapper0_cpu_read,
    .cpu_translate_write = mapper0_cpu_write,
    .ppu_translate_read = mapper0_ppu_read,
    .ppu_translate_write = mapper0_ppu_write
};

#endif /* MAPPERS_H */
