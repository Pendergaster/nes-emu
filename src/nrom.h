/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef NROM_H
#define NROM_H

// This file just implements the mapping functions

#define MAP0_START              0x8000
#define MAP0_END                0xFFFF
#define MAP0_PPU_DATA_SIZE      0x1FFF

void
mapper0_init(Mapper0Data* data, u8* progMem, u8* charMem) {

    data->programMemory = calloc(cartridge.numProgramRoms, PROG_ROM_SINGLE_SIZE);
    data->characterMemory = calloc(cartridge.numCharacterRoms, CHAR_ROM_SINGLE_SIZE);

    memcpy(data->programMemory, progMem,
            cartridge.numProgramRoms * PROG_ROM_SINGLE_SIZE);
    memcpy(data->characterMemory, charMem,
            cartridge.numCharacterRoms * CHAR_ROM_SINGLE_SIZE);
}

void
mapper0_dispose(Mapper0Data* data) {

    free(data->programMemory);
    free(data->characterMemory);

    memset(data, 0 ,sizeof *data);
}

u8
mapper0_cpu_peak(Mapper0Data* data, u16 addr, u8* valid) {

    if(!address_is_between(addr, MAP0_START, MAP0_END))  return 0;

    if(cartridge.numProgramRoms == 1)
        addr &= 0x3FFF; // if 1 rom capasity is 16K
    else
        addr &= 0x7FFF; // if 2 rom capasity is 32K

    u8 ret = data->programMemory[addr];

    if(valid) *valid = 1;
    return ret;
}

u8
mapper0_cpu_read(Mapper0Data* data, u16 addr) {

    if(!address_is_between(addr, MAP0_START, MAP0_END)) {
        return 0;
    }

    if(cartridge.numProgramRoms == 1)
        addr &= 0x3FFF; // if 1 rom capasity is 16K
    else
        addr &= 0x7FFF; // if 2 rom capasity is 32K

    return data->programMemory[addr];
}

void
mapper0_cpu_write(Mapper0Data* data, u16 addr, u8 val) {

    if(!address_is_between(addr, MAP0_START, MAP0_END)) {
        ABORT("invalid address in mapper0 0x%04X", addr);
        return;
    }

    if(cartridge.numProgramRoms == 1)
        addr &= 0x3FFF; // if 1 rom capasity is 16K
    else
        addr &= 0x7FFF; // if 2 rom capasity is 32K

    data->programMemory[addr] = val;
}

u8
mapper0_ppu_read(Mapper0Data* data, u16 addr) {

    if(!address_is_between(addr, 0, MAP0_PPU_DATA_SIZE))
        ABORT("invalid address in mapper0 0x%04X", addr);

    return data->characterMemory[addr];
}

void
mapper0_ppu_write(Mapper0Data* data, u16 addr, u8 val) {

    if(!address_is_between(addr, 0, MAP0_PPU_DATA_SIZE)) ABORT("invalid address in mapper0");

    data->characterMemory[addr] = val;
}

struct Mapper mapper0 = {
    .cpu_peak_cartridge     = (peak_func)mapper0_cpu_peak,
    .cpu_read_cartridge     = (cpu_read_func)mapper0_cpu_read,
    .cpu_write_cartridge    = (cpu_write_func)mapper0_cpu_write,
    .ppu_read_cartridge     = (ppu_read_func)mapper0_ppu_read,
    .ppu_write_cartridge    = (ppu_write_func)mapper0_ppu_write,
    .mapper_init            = (mapper_init_func)mapper0_init,
    .mapper_dispose         = (mapper_dispose_func)mapper0_dispose
};

#endif /* NROM_H */
