/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef MAPPERDATA_H
#define MAPPERDATA_H

char* notKnownOperand = "Outside of PRG mem";

typedef struct DisasseblyTable {
    char* disassebly; // PROG_ROM_SINGLE_SIZE of 20 sized blocks of strings
} DisasseblyTable ;

/* Common data to mapper */
typedef struct MapperHeader {

    u8*                 characterMemory;
    u32                 characterMemoryLen;

    u8*                 programMemory;
    u32                 programMemoryLen;

    // Table for each bank
    u32                 numPrgBanks;
    DisasseblyTable*    tables;
} MapperHeader;

typedef struct Mapper0Data {
    MapperHeader head;
} Mapper0Data;

#if 1
typedef enum MAP1ControllerReqData {
    Map1MirroringMode   = (0x3),
    Map1PRGBankMode     = (0xC),
    Map1CHRBankMode     = (0x10),
} MAP1ControllerReqData ;

typedef struct Mapper1Data {

    MapperHeader head;
    u8*     programRAM;         // $6000-$7FFF: 8 KB PRG RAM bank

    /*  How the shift reqister works
     *              ;  A          MMC1_SR  MMC1_PB
     * setPRGBank:  ;  000edcba    10000
     *  sta $E000   ;  000edcba -> a1000
     *  lsr a       ; >0000edcb    a1000
     *  sta $E000   ;  0000edcb -> ba100
     *  lsr a       ; >00000edc    ba100
     *  sta $E000   ;  00000edc -> cba10
     *  lsr a       ; >000000ed    cba10
     *  sta $E000   ;  000000ed -> dcba1
     *  lsr a       ; >0000000e    dcba1
     *  sta $E000   ;  0000000e    dcba1 -> edcba
     *              ;              10000
     */
    u8      shiftReqister;

    /* Controller Reqister
     * 4bit0
     * -----
     * CPPMM
     * |||||
     * |||++- Mirroring (0: one-screen, lower bank; 1: one-screen, upper bank;
     * |||               2: vertical; 3: horizontal)
     * |++--- PRG ROM bank mode (0, 1: switch 32 KB at $8000, ignoring low bit of bank number;
     * |                         2: fix first bank at $8000 and switch 16 KB bank at $C000;
     * |                         3: fix last bank at $C000 and switch 16 KB bank at $8000)
     * +----- CHR ROM bank mode (0: switch 8 KB at a time; 1: switch two separate 4 KB banks)
     **/
    u8      controlReqister;

    // Select 4 KB or 8 KB CHR bank at PPU $0000 (low bit ignored in 8 KB mode)
    u8      chrBank0reqister;
    // Select 4 KB CHR bank at PPU $1000 (ignored in 8 KB mode)
    u8      chrBank1reqister;

    /* PRG Bank
     * 4bit0
     * -----
     * RPPPP
     * |||||
     * |++++- Select 16 KB PRG ROM bank (low bit ignored in 32 KB mode)
     * +----- PRG RAM chip enable (0: enabled; 1: disabled; ignored on MMC1A)
     */
    u8      prgBankReqister;
} Mapper1Data;
#endif
#endif /* MAPPERDATA_H */
