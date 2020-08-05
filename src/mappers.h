/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef MAPPERS_H
#define MAPPERS_H

#include "mapperdata.h"

typedef struct DisasseblyTable {
    char** disassebly;
    //y8
} DisasseblyTable ;

static inline u32
create_instruction_str(char* str, u32 addr) {

    u8 opcode = bus_peak8(addr, NULL);
    Instruction instruction = instructionTable[opcode];
    u16 low = 0x0;
    u16 high = 0x0;

    u32 instLen = strlen(cpuInstructionStrings[opcode]);
    memcpy(str ,cpuInstructionStrings[opcode], instLen);

    str[instLen] = ' ';

    u32 skip = 0;

    if(instruction.addressMode == ABS ||                // fetch 2 addresses
            instruction.addressMode == ABSX ||
            instruction.addressMode == ABSY ||
            instruction.addressMode == IND
      ) {

        skip++;
        low = bus_peak8(addr + skip, NULL);

        skip++;
        high = bus_peak8(addr + skip, NULL);
    }
    else if( instruction.addressMode == IMP || // fetch 0 addresses
            instruction.addressMode == ACCUM //||
            //instruction.addressMode == IMM
           )
    {
        str[instLen + 1] = 0;
        return skip;
    }
    else // fetch low adress (1 address)
    {
        skip++;
        low = bus_peak8(addr + skip, NULL); // TODO fix
    }

    u16 holeAddr = (high << 8) | low;
    char temp[32];

    sprintf(temp, "0x%04X", holeAddr);

    int hexStringSize = strlen(temp) + 1;
    memcpy(&str[instLen + 1], temp, hexStringSize);

    return skip;
}

#include "nrom.h"
#include "mmc1.h"

#endif /* MAPPERS_H */
