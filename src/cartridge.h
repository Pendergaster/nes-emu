/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "fileload.h"

// Because we dont know before hand which mapper will be used
// this structure will store mapper functions

typedef enum MirrorType {
    HORIZONTAL,
    VERTICAL,
    ONESCREEN_LO, // TODO ??
    ONESCREEN_HI, // TODO ??
} MirrorType;

struct Cartridge {
    u8*         characterMemory;
    u8*         programMemory;

    u32         mapperID;
    u32         numProgramRoms;
    u32         numCharacterRoms;

    MirrorType  mirrorType;
} cartridge;

struct Mapper {
    u8      (*cpu_peak_cartridge)(u16 /*addr*/);
    u8      (*cpu_read_cartridge)(u16 /*addr*/);
    void    (*cpu_write_cartridge)(u16 /*addr*/, u8 /*val*/);
    u8      (*ppu_read_cartridge)(u16 /*addr*/);
    void    (*ppu_write_cartridge)(u16 /*addr*/, u8 /*val*/);
    u16     programMemStart;
} mapper;

#include "mappers.h"


// https://wiki.nesdev.com/w/index.php/INES
// https://formats.kaitai.io/ines/index.html
typedef struct INESHeader {
    u32     magic;
    u8      programRomCount;
    u8      charaterRomCount;
    u8      flag6;
    u8      flag7;
    u8      programRamLen;
    u8      flags9; //rarely used
    u8      flags10; //rarely used
    char    reserved[5];
} INESHeader;

STATIC_ASSERT(sizeof(INESHeader) == 16, header_size_wrong);

// INES fomrat contains
//
// Header (16 bytes)
// Trainer, if present (0 or 512 bytes)
// PRG ROM data (16384 * x bytes)
// CHR ROM data, if present (8192 * y bytes)
// PlayChoice INST-ROM, if present (0 or 8192 bytes)
// PlayChoice PROM, if present (16 bytes Data, 16 bytes CounterOut)
//                  (this is often missing, see PC10 ROM-Images for details)

#define PROG_ROM_SINGLE_SIZE    16384
#define CHAR_ROM_SINGLE_SIZE    8192
#define TRAINER_SIZE            512

static void
cartridge_load(const char* name) {
    size_t size;
    u8* to_free;
    u8* data = to_free = load_binary_file(name, &size);
    if(!data) {
        ABORT("failed to load cartridge");
    }

    INESHeader header;
    memcpy( &header, data, sizeof(INESHeader));

    data += sizeof(INESHeader);
    // next 512 bytes is trainer //TODO
    if (header.flag6 & 0x04) {
        data += TRAINER_SIZE;
    }

    //int high = header.flag7; // TODO check
    //int low = header.flag6;

    cartridge.mapperID =  ((header.flag6 & 0xF0) >> 4) | (header.flag7 & 0xF0);

    cartridge.mirrorType = header.flag6 & 0x1 ? VERTICAL : HORIZONTAL;

    // (high << 4) | low;

    // type 1 file format TODO rest of them

    cartridge.numProgramRoms = header.programRomCount;
    cartridge.numCharacterRoms = header.charaterRomCount;

    cartridge.programMemory = calloc(cartridge.numProgramRoms, PROG_ROM_SINGLE_SIZE);
    cartridge.characterMemory = calloc(cartridge.numCharacterRoms, CHAR_ROM_SINGLE_SIZE);

    memcpy(cartridge.programMemory, data, cartridge.numProgramRoms * PROG_ROM_SINGLE_SIZE);
    data += cartridge.numProgramRoms * PROG_ROM_SINGLE_SIZE;
    memcpy(cartridge.characterMemory, data, cartridge.numCharacterRoms * CHAR_ROM_SINGLE_SIZE);
    data += cartridge.numCharacterRoms * CHAR_ROM_SINGLE_SIZE;

    // print signature
    char signatureName[4] = {};
    memcpy(signatureName, &header.magic, sizeof(u32));
    LOG("Cartridge signature %s", signatureName);
    LOG("CHR ROM %d", header.charaterRomCount);
    LOG("PRG ROM %d", header.programRomCount);

    switch(cartridge.mapperID) {

        case 0:
            mapper = mapper0;
            break;
        default:
            ABORT("NOT IMPLEMENTED MAPPER %d", cartridge.mapperID);
            break;

    }

    free(to_free);
}

u8
cartridge_peak(u16 addr) {

    u16 actualAddr = mapper.cpu_read_cartridge(addr);

    if(actualAddr == 0xFFFF) return 0xFF;

    if(actualAddr >= cartridge.numProgramRoms * PROG_ROM_SINGLE_SIZE) ABORT("mem overflow");

    u8 data = cartridge.programMemory[actualAddr];

    return data;
}

static inline u8
cartridge_cpu_read_rom(u16 addr) {

    return mapper.cpu_read_cartridge(addr);
}


static inline void
cartridge_cpu_write_rom(u16 addr, u8 val) {

    mapper.cpu_write_cartridge(addr, val);
}


static inline u8
cartridge_ppu_read_rom(u16 addr) {

    return mapper.ppu_read_cartridge(addr);
}

void
cartridge_ppu_write_rom(u16 addr, u8 val) {

    mapper.ppu_write_cartridge(addr, val);
}

#endif /* CARTRIDGE_H */
