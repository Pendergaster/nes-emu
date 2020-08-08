/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef MAPPERS_H
#define MAPPERS_H

#include "mapperdata.h"
#include "cpudata.h"

static inline void
disassemblytable_write(MapperHeader* data, u32 addr /*prg mem space*/, char* str /*20 size*/) {

    u32 bank = (u32)addr / PROG_ROM_SINGLE_SIZE;
    ASSERT_MESSAGE(bank < data->numPrgBanks, "Failed to write to disassembly table 0x%04X", addr);

    u32 index = addr % PROG_ROM_SINGLE_SIZE;
    ASSERT_MESSAGE(index < PROG_ROM_SINGLE_SIZE, "Failed to write to disassembly table 0x%04X", addr);
    index *= 20;

    memcpy(&data->tables[bank].disassebly[index], str, sizeof(char) * 20);
}

static DisasseblyTable*
disassemblytables_get(u32 banks) {

    u32 datasize = sizeof(char) * 20 * PROG_ROM_SINGLE_SIZE;
    DisasseblyTable* ret = calloc( (sizeof(DisasseblyTable) + datasize) * banks, 1);

    char* datastart = (char*)&ret[banks];
    for(u32 i = 0; i < banks; i++) {
        ret[i].disassebly = datastart + (datasize * i);
    }

    return ret;
}

static inline char*
disassemblytable_read(MapperHeader* data, u32 addr) {

    u32 bank = (u32)addr / PROG_ROM_SINGLE_SIZE;
    ASSERT_MESSAGE(bank < data->numPrgBanks, "Failed to read from disassembly table 0x%04X", addr);
    u32 index = addr % PROG_ROM_SINGLE_SIZE;
    ASSERT_MESSAGE(index < PROG_ROM_SINGLE_SIZE, "Failed to read from disassembly table 0x%04X", addr);
    index *= 20;

    return &data->tables[bank].disassebly[index];
}


static void
mapperheader_init(MapperHeader* data, u8* progMem, u8* charMem) {

    data->programMemory = calloc(cartridge.numProgramRoms, PROG_ROM_SINGLE_SIZE);
    data->programMemoryLen = cartridge.numProgramRoms * PROG_ROM_SINGLE_SIZE;

    if(cartridge.numCharacterRoms) {
        data->characterMemory = calloc(cartridge.numCharacterRoms, CHAR_ROM_SINGLE_SIZE);
        data->characterMemoryLen = cartridge.numCharacterRoms * CHAR_ROM_SINGLE_SIZE;
    } else {
        data->characterMemory = calloc(1, CHAR_ROM_SINGLE_SIZE);
        data->characterMemoryLen = CHAR_ROM_SINGLE_SIZE;
    }

    memcpy(data->programMemory, progMem,
            cartridge.numProgramRoms * PROG_ROM_SINGLE_SIZE);
    memcpy(data->characterMemory, charMem,
            cartridge.numCharacterRoms * CHAR_ROM_SINGLE_SIZE);

    data->numPrgBanks = cartridge.numProgramRoms;

    data->tables = disassemblytables_get(data->numPrgBanks);

    /* init disassemblytable */
    char codestr[20];
    for(u32 i = 0; i < data->programMemoryLen; i++) {

        u32 pos = i;
        u8 opcode = data->programMemory[i];

        Instruction instruction = instructionTable[opcode];
        u16 low = 0x0;
        u16 high = 0x0;

        u32 instLen = strlen(cpuInstructionStrings[opcode]);
        memcpy(codestr ,cpuInstructionStrings[opcode], instLen);

        codestr[instLen] = ' ';

        if(instruction.addressMode == ABS ||                // fetch 2 addresses
                instruction.addressMode == ABSX ||
                instruction.addressMode == ABSY ||
                instruction.addressMode == IND) {

            i += 1;
            if(i >= data->programMemoryLen) break;
            low = data->programMemory[i];

            i += 1;
            if(i >= data->programMemoryLen) break;
            high = data->programMemory[i];
        } else if( instruction.addressMode == IMP || instruction.addressMode == ACCUM) {
            /* fetch 0 addresses */
            codestr[instLen + 1] = 0;
            // write disassembled instruction
            disassemblytable_write(data, pos, codestr);
            continue;
        }

        else // fetch low adress (1 address)
        {
            i += 1;
            if(i >= data->programMemoryLen) break;
            low = data->programMemory[i];
        }

        u16 holeAddr = (high << 8) | low;
        char temp[32];

        sprintf(temp, "0x%04X", holeAddr);

        int hexStringSize = strlen(temp) + 1;
        memcpy(&codestr[instLen + 1], temp, hexStringSize);

        disassemblytable_write(data, pos, codestr);
    }
}

void
mapperheader_dispose(MapperHeader* data) {

    free(data->programMemory);
    free(data->characterMemory);
    free(data->tables);

    memset(data, 0 ,sizeof *data);
}


#include "nrom.h"
#include "mmc1.h"

#endif /* MAPPERS_H */
