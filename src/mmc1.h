/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef MMC1_H
#define MMC1_H


#define MAP1_RAM_START          0x6000
#define MAP1_RAM_END            0x7FFF
#define MAP1_RAM_SIZE           0x2000

#define MAP1_CONTROL_START      0x8000
#define MAP1_CONTROL_END        0x9FFF

#define MAP1_CHRBANK0_START     0xA000
#define MAP1_CHRBANK0_END       0xBFFF

#define MAP1_CHRBANK1_START     0xC000
#define MAP1_CHRBANK1_END       0xDFFF

#define MAP1_PRGBANK_START      0xE000
#define MAP1_PRGBANK_END        0xFFFF

void
mapper1_init(Mapper1Data* data, u8* progMem, u8* charMem) {

    mapperheader_init(&data->head, progMem, charMem);

    data->programRAM = calloc(MAP1_RAM_SIZE, 1);
    data->controlReqister = 0xF;
}

u32 _mapper1_get_prg_addr(Mapper1Data* data, u16 addr) {

    if(addr < MAP1_CONTROL_START ) return numeric_max_u32;

    u8 prgRomMode = (data->controlReqister & Map1PRGBankMode) >> 2;

    switch (prgRomMode) {
        case 0: case 1:
            {
                // switch 32 KB at $8000, ignoring low bit of bank number
                u8 bank = data->prgBankReqister & 0xE;
                u32 address = (addr - 0x8000) + (bank * 0x8000 /*32kb*/);

                //LOG("addr 0x%04X", address);

                ASSERT_MESSAGE(address < data->head.programMemoryLen, "MMC1 prg mem invalid address");
                return address;
            } break;
        case 2:
            {
                // fix first bank at $8000 and switch 16 KB bank at $C000
                if(addr >= 0xC000) {
                    u32 address = addr - 0x8000;
                    ASSERT_MESSAGE(address < data->head.programMemoryLen, "MMC1 prg mem invalid address");
                    return address;
                } else {
                    u8 bank = data->prgBankReqister;
                    u32 address = (addr - 0xC000) + (bank * 0x4000 /*16kb*/);
                    ASSERT_MESSAGE(address < data->head.programMemoryLen, "MMC1 prg mem invalid address");
                    return address;
                }
            } break;
        case 3:
            {
                // fix last bank at $C000 and switch 16 KB bank at $8000
                if(addr >= 0xC000) {
                    // last bank
                    u32 address = (data->head.programMemoryLen - PROG_ROM_SINGLE_SIZE) + (addr - 0xC000);
                    ASSERT_MESSAGE(address < data->head.programMemoryLen, "MMC1 prg mem invalid address");
                    return address;
                } else {
                    u8 bank = data->prgBankReqister;
                    u32 address = (addr - 0x8000) + (bank * 0x4000 /*16kb*/);
                    ASSERT_MESSAGE(address < data->head.programMemoryLen, "MMC1 prg mem invalid address");
                    return address;
                }

            } break;
        default:
            ABORT("MMC1 unknown PRG ROM mode 0x%04X", prgRomMode);
            break;
    }

    return numeric_max_u32;
}

char*
mapper1_disassemble(Mapper1Data* data, u16 addr) {

    u32 address =_mapper1_get_prg_addr(data, addr);
    if(address == numeric_max_u32) return notKnownOperand;
    return disassemblytable_read(&data->head, address);
}


void
mapper1_dispose(Mapper1Data* data) {

    mapperheader_dispose(&data->head);
    free(data->programRAM);

    memset(data, 0 ,sizeof *data);
}

u8
mapper1_cpu_read(Mapper1Data* data, u16 addr) {

    if(addr < MAP1_RAM_START ) return 0; // Nova the squirrel fix

    if(address_is_between(addr, MAP1_RAM_START , MAP1_RAM_END)) {

        // ram enabled?
        if (!(data->prgBankReqister & 0x10)) {
            return data->programRAM[addr - 0x6000];
        }
        return 0;
    }

    u32 address =_mapper1_get_prg_addr(data, addr);
    if(address == numeric_max_u32) ABORT("MMC1 prg mem invalid address 0x%04X", address);
    return data->head.programMemory[address];
}

u8
mapper1_cpu_peak(Mapper1Data* data, u16 addr, u8* valid) {

    if(addr >= MAP1_CONTROL_START) {
        if(valid) *valid = 1;
        return mapper1_cpu_read(data, addr);
    }
    return 0;
}

void
mapper1_cpu_write(Mapper1Data* data, u16 addr, u8 val) {

    if(address_is_between(addr, MAP1_RAM_START , MAP1_RAM_END)) {

        // ram enabled?
        if (!(data->prgBankReqister & 0x10)) {
            data->programRAM[addr - 0x6000] = val;
        }
        return;
    }

    ASSERT_MESSAGE(addr >= 0x8000, "invalid address in MMC1 mapper");

    // $8000-$FFFF area
    // Writing a value with bit 7 set ($80 through $FF) to any address in $8000-$FFFF
    // clears the shift register to its initial state.
    if(val & 0x80) {
        // TODO set control?

        data->controlReqister |= 0xC;
        data->shiftReqister = 0x10;
        return;
    }

    if(data->shiftReqister & 0x1) {
        // shift reqister has been shifted 4 times (fifth write)

        data->shiftReqister >>= 1;
        data->shiftReqister |= (val & 1) << 4;

        if(address_is_between(addr, MAP1_CONTROL_START, MAP1_CONTROL_END)) {
            data->controlReqister = data->shiftReqister;
            u8 mirroringMode = data->controlReqister & Map1MirroringMode;

            switch(mirroringMode) {
                case 0:
                    {
                        cartridge.mirrorType = ONESCREEN_LO;
                    } break;
                case 1:
                    {
                        cartridge.mirrorType = ONESCREEN_HI;
                    } break;
                case 2:
                    {
                        cartridge.mirrorType = VERTICAL;
                    } break;
                case 3:
                    {
                        cartridge.mirrorType = HORIZONTAL;
                    } break;
                default:
                    ABORT("MMC1 mirroring mode not supported");
                    break;
            }
        } else if(address_is_between(addr, MAP1_CHRBANK0_START, MAP1_CHRBANK0_END)) {
            data->chrBank0reqister = data->shiftReqister;
        } else if(address_is_between(addr, MAP1_CHRBANK1_START, MAP1_CHRBANK1_END)) {
            data->chrBank1reqister = data->shiftReqister;
        } else if(address_is_between(addr, MAP1_PRGBANK_START, MAP1_PRGBANK_END)) {
            data->prgBankReqister = data->shiftReqister;
        } else {
            ABORT("MMC1 address range not covered");
        }


        data->shiftReqister = 0x10;
    } else {
        // To change a register's value, the CPU writes five times with bit 7 clear
        // and a bit of the desired value in bit 0.
        // On the first four writes, the MMC1 shifts bit 0 into a shift register.
        // On the fifth write, the MMC1 copies bit 0 and the shift register contents
        // into an internal register selected by bits 14 and 13 of the address,
        // and then it clears the shift register

        data->shiftReqister >>= 1;
        data->shiftReqister |= (val & 1) << 4;
    }
}

u8
mapper1_ppu_read(Mapper1Data* data, u16 addr) {

    u8 chrBankMode = (data->controlReqister & Map1CHRBankMode) >> 0x4;

    //8kb mode
    if(chrBankMode == 0) {
        // ignore 0 bit
        u8 bank = data->chrBank0reqister & 0x1E;
        u16 address = (bank * 0x2000 /*8 kb*/) + addr;
        ASSERT_MESSAGE(address < data->head.characterMemoryLen, "MMC1 prg mem invalid address");
        return data->head.characterMemory[address];
        //4kb mode
    } else {

        if(addr < 0x1000) {
            u16 address = data->chrBank0reqister * 0x1000 /*4 kb*/ + addr;
            ASSERT_MESSAGE(address < data->head.characterMemoryLen, "MMC1 prg mem invalid address");
            return data->head.characterMemory[address];
        } else {
            u16 address = (data->chrBank1reqister * 0x1000 /*4 kb*/)  + (addr - 0x1000 /*4 kb*/);
            ASSERT_MESSAGE(address < data->head.characterMemoryLen, "MMC1 prg mem invalid address");
            return data->head.characterMemory[address];
        }
    }
}

void
mapper1_ppu_write(Mapper1Data* data, u16 addr, u8 val) {

    ASSERT_MESSAGE(addr < data->head.characterMemoryLen, "invalid write in mmc1");
    data->head.characterMemory[addr] = val;
}

struct Mapper mapper1 = {
    .cpu_read_cartridge     = (cpu_read_func)mapper1_cpu_read,
    .cpu_write_cartridge    = (cpu_write_func)mapper1_cpu_write,
    .ppu_read_cartridge     = (ppu_read_func)mapper1_ppu_read,
    .ppu_write_cartridge    = (ppu_write_func)mapper1_ppu_write,
    .mapper_init            = (mapper_init_func)mapper1_init,
    .mapper_dispose         = (mapper_dispose_func)mapper1_dispose,
    .cpu_peak_cartridge     = (peak_func)mapper1_cpu_peak,
    .mapper_disasseble      = (disasseble_func)mapper1_disassemble
};

#endif /* MMC1_H */
