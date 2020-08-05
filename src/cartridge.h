/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "fileload.h"
#include "mapperdata.h"

#define PROG_ROM_SINGLE_SIZE    0x4000 // 16 K
#define CHAR_ROM_SINGLE_SIZE    0x2000 // 8 K
#define TRAINER_SIZE            512

// Because we dont know before hand which mapper will be used
// this structure will store mapper functions

typedef enum MirrorType {
    HORIZONTAL,
    VERTICAL,
    ONESCREEN_LO, // TODO ??
    ONESCREEN_HI, // TODO ??
} MirrorType;

struct Cartridge {
    u32         mapperID;
    u32         numProgramRoms;
    u32         numCharacterRoms;
    MirrorType  mirrorType;
} cartridge;

typedef union MapperData MapperData;

typedef u8   (*peak_func)(MapperData* /*data*/, u16 /*addr*/, u8* /*valid*/);
typedef u8   (*cpu_read_func)(MapperData* /*data*/, u16 /*addr*/);
typedef void (*cpu_write_func)(MapperData* /*data*/, u16 /*addr*/, u8 /*val*/);
typedef u8   (*ppu_read_func)(MapperData* /*data*/, u16 /*addr*/);
typedef void (*ppu_write_func)(MapperData* /*data*/, u16 /*addr*/, u8 /*val*/);
typedef void (*mapper_init_func)(MapperData* /*data*/, u8* progMem, u8* charMem);
typedef void (*mapper_dispose_func)(MapperData* /*data*/);

union MapperData {
    Mapper0Data mapper0;
    Mapper1Data mapper1;
};

struct Mapper {
    peak_func           cpu_peak_cartridge;
    cpu_read_func       cpu_read_cartridge;
    cpu_write_func      cpu_write_cartridge;
    ppu_read_func       ppu_read_cartridge;
    ppu_write_func      ppu_write_cartridge;
    mapper_init_func    mapper_init;
    mapper_dispose_func mapper_dispose;
    MapperData          data;
} mapper;

#include "mappers.h"


//STATIC_ASSERT(sizeof(union MapperData) == sizeof(u8*), mapper_union_size_wrong);

// https://wiki.nesdev.com/w/index.php/INES
// https://formats.kaitai.io/ines/index.html
typedef struct INESHeader {
    u32     magic;
    u8      programRomCount;
    u8      charaterRomCount;
    u8      flag6;
    u8      flag7;
    u8      programRamLen;
    u8      flags9;  // Rarely used
    u8      flags10; // Rarely used
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

    // type 1 file format TODO rest of them ??

    cartridge.numProgramRoms = header.programRomCount;
    cartridge.numCharacterRoms = header.charaterRomCount;

    ASSERT_MESSAGE(cartridge.numProgramRoms, "No PRG roms detected");

    u8* programMemory = data;
    data += cartridge.numProgramRoms * PROG_ROM_SINGLE_SIZE;
    u8* characterMemory = data;
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
        case 1:
            mapper = mapper1;
            break;
        default:
            ABORT("NOT IMPLEMENTED MAPPER %d", cartridge.mapperID);
            break;
    }

    if(mapper.mapper_init) mapper.mapper_init(&mapper.data, programMemory, characterMemory);

    free(to_free);
}

void
cartridge_dispose() {
    if(mapper.mapper_dispose) mapper.mapper_dispose(&mapper.data);
}

u8
cartridge_peak(u16 addr, u8* valid) {

    return mapper.cpu_peak_cartridge(&mapper.data, addr, valid);
}

static inline u8
cartridge_cpu_read_rom(u16 addr) {

    return mapper.cpu_read_cartridge(&mapper.data, addr);
}


static inline void
cartridge_cpu_write_rom(u16 addr, u8 val) {

    mapper.cpu_write_cartridge(&mapper.data, addr, val);
}


static inline u8
cartridge_ppu_read_rom(u16 addr) {

    return mapper.ppu_read_cartridge(&mapper.data, addr);
}

void
cartridge_ppu_write_rom(u16 addr, u8 val) {

    mapper.ppu_write_cartridge(&mapper.data, addr, val);
}

#endif /* CARTRIDGE_H */
