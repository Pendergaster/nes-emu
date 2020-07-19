/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

static u8 ppu_read(u16 addr);

#ifndef PPU_H
#define PPU_H

#define NAMETABLE_SIZE                  0x400 // 1024
#define TEX_HEIGHT                      240
#define TEX_WIDTH                       256

#define TILE_DIM                        8
#define NUM_TILES                       16

#define PPU_MAX_MEMORY_ADDR             0x3FFF

#define PPU_PATTERN_MEMORY_START        0x0
#define PPU_PATTERN_MEMORY_END          0x1FFF

#define PPU_NAMETABLE_MEMORY_START      0x2000
#define PPU_NAMETABLE_MEMORY_END        0x3EFF

#define PPU_PALETTE_MEMORY_START        0x3F00
#define PPU_PALETTE_MEMORY_END          0x3FFF

#define PPU_DMA_WRITE_ADDRESS           0x4014

#include "cpudata.h"

typedef struct ImageView {
    u8*     data;
    u32     w,h;
    GLuint  tex;
} ImageView;

// http://wiki.nesdev.com/w/index.php/PPU_registers
typedef enum PPUStatus {
    SpriteOverflow       = (1 << 5),
    Sprite0Hit           = (1 << 6),
    VerticalBlankStarted = (1 << 7),
} PPUStatus ;

typedef enum PPUMask {
    GreyScale             = (1 << 0),
    BackgroungIn8MostLeft = (1 << 1),
    SpriteIn8MostLeft     = (1 << 2),
    ShowBackground        = (1 << 3),
    ShowSprites           = (1 << 4),
    EmphasizeRed          = (1 << 5),
    EmphasizeGreen        = (1 << 6),
    EmphasizeBlue         = (1 << 7)
} PPUMask ;

typedef enum PPUController {
    BaseNameTableAddress1     = (1 << 0),
    BaseNameTableAddress2     = (1 << 1),
    VramAddressIncrement      = (1 << 2), // (0: add 1, going across; 1: add 32, going down)
    SpritePatterntableAddress = (1 << 3),
    BackgroundTableAddress    = (1 << 4),
    SpriteSize                = (1 << 5), // Sprite size (0: 8x8 pixels; 1: 8x16 pixels)
    PPUMasterSlaveSelect      = (1 << 6), // 0: read backdrop from EXT pins;
    // 1: output color on EXT pins
    GenerateNMI               = (1 << 7)
} PPUController ;

//https://wiki.nesdev.com/w/index.php/PPU_scrolling

//   First        Second
//   |---------| |-------|
//   0 0yy NN YY YYY XXXXX
//     ||| || || ||| +++++-- coarse X scroll
//     ||| || ++-+++-------- coarse Y scroll
//     ||| ++--------------- nametable select
//     +++------------------ fine Y scroll
typedef union Loopy {
    struct {
        u16 coarseX : 5;
        u16 coarseY : 5;
        u16 nametableSelect : 2; // x and y nametables
        u16 fineY : 3;
    };

    u16 reqister;
} Loopy ;

typedef enum DMAstate {
    DMANotActive        = 0,
    DMAWaitingForCopy   = 1,
    DMACopyingActive    = 2,
} DMAstate;

STATIC_ASSERT(sizeof(Loopy) == sizeof(u16), loopy_size_wrong);

typedef enum OAMDataAttributes {
    SpritePaletteLow      = (1 << 0),
    SpritePaletteHigh     = (1 << 1),
    SpritePriority        = (1 << 5),
    SpriteHorizontalFlip  = (1 << 6),
    SpriteVerticalFlip    = (1 << 7),
} OAMDataAttributes;

typedef struct OAMData {
    u8 yPos;
    u8 tileIndex;
    u8 attributes;
    u8 xPos;
} OAMData;

STATIC_ASSERT(sizeof(OAMData) == sizeof(u32), OAMData_size_wrong);

typedef struct OAM {

    u8          primary[256];                // 64 sprites
    u8          secondary[32];               // 8 sprites
    u8          numSpritesFound;
    u8          xCounters[8];
    u8          attributeLatches[8];

    u8          addr;
    u8          DMAactive;
    u8          DMAaddr; // lets assume that reading starts at XX00 address always //TODO ??

    //          indicates if 0x00 sprite is in secondaryOAM
    u8          spriteZeroRendered;
} OAM;

struct PPU {
    u8          nameTables[2 * NAMETABLE_SIZE]; // layout of background 0x2000 - 0x3F00
    //u8          patternTables[2][4096];         // sprites 0x0 - 0x1FFF
    u8          palette[32];                    // color palette 0x3F00 - 0x3FFF

    OAM         oam;
    // Viewable variables
    //u8      paletteView[0x40];
    //u8*     screenSprite;
    //u8*     nameTableSprites[2];
    //u8*     patternTableSprites[2];
    // pixelY
    i16         scanline;
    // column
    i16         cycle;
    u8          frameComplete;

    u8          statusReq;
    u8          maskReq;
    u8          controllerReq;

    u8          dataAddrAccess;
    u8          internalDataBuffer;

    Loopy       loopyT; // Write reqister
    Loopy       loopyV;

    // Pixel offset
    uint8_t     fineX;

    // https://wiki.nesdev.com/w/images/d/d1/Ntsc_timing.png
    // these are fetched for the rendering
    u8          NTbyte;
    u8          ATbyte;
    u8          LowBGbyte;
    u8          HighBGbyte;

    u16         shifterLow;
    u16         shifterHigh;
    u16         paletteShifterHigh;
    u16         paletteShifterLow;

    u8          spriteLowShifter[8];
    u8          spriteHighShifter[8];

    u8          NMIGenerated; // TODO sould generate one in cpu write?
    // http://wiki.nesdev.com/w/index.php/PPU_registers#PPUCTRL

    ImageView   screen;
    ImageView   pattern[2];
    ImageView   OAMvisualisation;

    GLuint      renderProgram;
    GLint       transformLoc;
    GLint       projectionLoc;
    GLuint      vao;
} ppu;

typedef struct Color {
    u8 r,g,b,a; // A is unused here (used only for aligning)
} Color;

STATIC_ASSERT(sizeof(Color) == sizeof(u32), color_size_wrong);

Color colors[0x40] = {
    {84, 84, 84, 1}, {0, 30, 116, 1}, {8, 16, 144, 1}, {48, 0, 136, 1}, {68, 0, 100, 1},
    {92, 0, 48, 1}, {84, 4, 0, 1}, {60, 24, 0, 1}, {32, 42, 0, 1}, {8, 58, 0, 1}, {0, 64, 0, 1},
    {0, 60, 0, 1}, {0, 50, 60, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {152, 150, 152, 1},
    {8, 76, 196, 1}, {48, 50, 236, 1}, {92, 30, 228, 1}, {136, 20, 176, 1}, {160, 20, 100, 1},
    {152, 34, 32, 1}, {120, 60, 0, 1}, {84, 90, 0, 1}, {40, 114, 0, 1}, {8, 124, 0, 1},
    {0, 118, 40, 1}, {0, 102, 120, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {236, 238, 236, 1},
    {76, 154, 236, 1}, {120, 124, 236, 1}, {176, 98, 236, 1}, {228, 84, 236, 1}, {236, 88, 180, 1},
    {236, 106, 100, 1}, {212, 136, 32, 1}, {160, 170, 0, 1}, {116, 196, 0, 1}, {76, 208, 32, 1},
    {56, 204, 108, 1}, {56, 180, 204, 1}, {60, 60, 60, 1}, {0, 0, 0, 1}, {0, 0, 0, 1},
    {236, 238, 236, 1}, {168, 204, 236, 1}, {188, 188, 236, 1}, {212, 178, 236, 1},
    {236, 174, 236, 1}, {236, 174, 212, 1}, {236, 180, 176, 1}, {228, 196, 144, 1},
    {204, 210, 120, 1}, {180, 222, 120, 1}, {168, 226, 144, 1}, {152, 226, 180, 1},
    {160, 214, 228, 1}, {160, 162, 160, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}
};

static u8
ppu_read(u16 addr) {

    u8 data = 0x00;
    addr &= PPU_MAX_MEMORY_ADDR;

    if(address_is_between(addr,
                PPU_PATTERN_MEMORY_START, PPU_PATTERN_MEMORY_END)) {

#if 0
        i8 table = (addr & 0x1000) >> 12;
        i8 tableIndex = addr & 0x0FFF;

        data = ppu.patternTables[table][tableIndex];
        // TODO fix or sth
#endif
        data = cartridge_ppu_read_rom(addr);

    } else if (address_is_between(addr,
                PPU_NAMETABLE_MEMORY_START, PPU_NAMETABLE_MEMORY_END)) {

        switch(cartridge.mirrorType){
            case VERTICAL:
                {
                    //[0][1]
                    //[0][1]

                    addr &= 0x0FFF;
                    u16 readAddr = addr % NAMETABLE_SIZE;

                    if(addr < NAMETABLE_SIZE) {
                        data = ppu.nameTables[readAddr];
                    } else if(addr < NAMETABLE_SIZE * 2) {
                        data = ppu.nameTables[NAMETABLE_SIZE + readAddr];
                    } else if(addr < NAMETABLE_SIZE * 3) {
                        data = ppu.nameTables[readAddr];
                    } else {
                        data = ppu.nameTables[NAMETABLE_SIZE + readAddr];
                    }
                } break;
            case HORIZONTAL:
                {
                    //[0][0]
                    //[1][1]
                    addr &= 0x0FFF;
                    u16 readAddr = addr % NAMETABLE_SIZE;
                    if(addr < NAMETABLE_SIZE * 2) {
                        data =  ppu.nameTables[readAddr];

                    } else {
                        data =  ppu.nameTables[NAMETABLE_SIZE + readAddr];
                    }
                } break;
            default:
                ABORT("unknown mirroring type");

        }

    } else if (address_is_between(addr,
                PPU_PALETTE_MEMORY_START, PPU_PALETTE_MEMORY_END)) {

        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004; // TODO check
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;

        data = ppu.palette[addr] & 0x3F;

    } else {
        data = cartridge_ppu_read_rom(addr);
    }

    return data;
}

static void
ppu_write(u16 addr, u8 data) {

    addr &= PPU_MAX_MEMORY_ADDR;

    if(address_is_between(addr,
                PPU_PATTERN_MEMORY_START, PPU_PATTERN_MEMORY_END)) {
#if 0
        i8 table = (addr & 0x1000) >> 12;
        i8 tableIndex = addr & 0x0FFF;

        //ppu.patternTables[table][tableIndex] = data;
#endif
        cartridge_ppu_write_rom(addr, data);

    } else if (address_is_between(addr,
                PPU_NAMETABLE_MEMORY_START, PPU_NAMETABLE_MEMORY_END)) {

        switch(cartridge.mirrorType){
            case VERTICAL:
                {
                    //[0][1]
                    //[0][1]

                    addr &= 0x0FFF;
                    u16 readAddr = addr % NAMETABLE_SIZE;

                    if(addr < NAMETABLE_SIZE) {
                        ppu.nameTables[readAddr] = data ;
                    } else if(addr < NAMETABLE_SIZE * 2) {
                        ppu.nameTables[NAMETABLE_SIZE + readAddr] = data;
                    } else if(addr < NAMETABLE_SIZE * 3) {
                        ppu.nameTables[readAddr] = data;
                    } else {
                        ppu.nameTables[NAMETABLE_SIZE + readAddr] = data;
                    }
                } break;
            case HORIZONTAL:
                {

                    //[0][0]
                    //[1][1]
                    addr &= 0x0FFF;
                    u16 readAddr = addr % NAMETABLE_SIZE;

                    if(addr < NAMETABLE_SIZE * 2) {
                        ppu.nameTables[readAddr] = data;
                    } else {
                        ppu.nameTables[NAMETABLE_SIZE + readAddr] = data;
                    }
                } break;
            default:
                ABORT("unknown mirroring type");

        }

    } else if (address_is_between(addr,
                PPU_PALETTE_MEMORY_START, PPU_PALETTE_MEMORY_END)) {

        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004; // TODO check
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;

        ppu.palette[addr] = data;
    } else {
        LOG("TODO ERROR");
    }
}

static inline void
load_shifters() {
    ppu.shifterLow = (ppu.shifterLow & 0xFF00) | ppu.LowBGbyte;
    ppu.shifterHigh = (ppu.shifterHigh & 0xFF00) | ppu.HighBGbyte;

    ppu.paletteShifterLow = (ppu.paletteShifterLow & 0xFF00) | (ppu.ATbyte & 0x1 ? 0x00FF : 0x0);
    ppu.paletteShifterHigh = (ppu.paletteShifterHigh & 0xFF00) | (ppu.ATbyte & 0x2 ? 0x00FF : 0x0);

    // TODO update palette
}

static inline Color
ppu_palette_get_color(u8 pixel, u32 paletteIndex) {

    u16 paletteLocation = PPU_PALETTE_MEMORY_START + (paletteIndex * 4) + pixel;
    u8 data = ppu_read(paletteLocation);

    return colors[data & 0x3F];
}

static void
ppu_dma_oam(u32 systemClock) {

    switch (ppu.oam.DMAactive) {
        case DMAWaitingForCopy:
            {
                if(systemClock % 2 == 1) { // Start copying on odd system clock
                    ppu.oam.DMAactive = DMACopyingActive;
                }
            } break;
        case DMACopyingActive:
            {
                // Copy the hole thing at once
                for(u32 i = 0; i <= 0xFF; i++) {
                    ppu.oam.primary[i] = bus_read8((ppu.oam.DMAaddr << 8) | i);
                }
#if 0
                OAMData* data = (OAMData*)ppu.oam.primary;
                LOG("");
                for(u32 i = 0; i < 64; i++) {
                    LOG("num %d DATA :%d %d %d %d", i,
                            data[i].yPos, data[i].xPos, data[i].attributes, data[i].tileIndex);
                }
#endif
                ppu.oam.DMAactive = DMANotActive;
                cpu.cycles += 0xFF * 2;
            } break;
    }
}

static void
ppu_oam_fetch_sprites() {

    memset(ppu.oam.secondary, 0xFF, sizeof(ppu.oam.secondary));

    // try to fetch 8 sprites
    u32 numFetched = 0;
    OAMData* evalSprites = (OAMData*)ppu.oam.primary;
    OAMData* fillArray = (OAMData*)ppu.oam.secondary;

    ppu.oam.spriteZeroRendered = 0;

    for(u32 i = 0; i < 64; i++) {

        OAMData sprite = evalSprites[i];
        i32 diff = (i32)ppu.scanline - (i32)sprite.yPos;
        i32 size = ppu.controllerReq & SpriteSize ? 16 : 8;

        if(diff < size && diff >= 0) {
            if(numFetched == 8) {
                ppu.statusReq |= SpriteOverflow;
                break;
            } else {
                // Add to secondary OAM
                fillArray[numFetched] = sprite;
                ppu.oam.xCounters[numFetched] = sprite.xPos;

                ppu.oam.attributeLatches[numFetched] = sprite.attributes;

                if(i == 0) {
                    ppu.oam.spriteZeroRendered = 1;
                }
            }
            numFetched += 1;
        }
    }
    ppu.oam.numSpritesFound = numFetched;
}

static void
ppu_load_sprite_shifters() {
    // populate sprite shifters
    OAMData* sprites = (OAMData*)ppu.oam.secondary;
    for(u32 i = 0; i < ppu.oam.numSpritesFound; i++) {

        u16 spriteAddr = 0;
        if(ppu.controllerReq & SpriteSize) { // 8x16

            // For 8x16 sprites, the PPU ignores the pattern table selection
            // and selects a pattern table from bit 0 of this number.
            u16 addressOffset = sprites[i].tileIndex & 0x1 ?  0x1000 : 0;
            // filter first bit, it is the patterntable choice
            spriteAddr = (sprites[i].tileIndex & 0xFE) * 16 + addressOffset;
            u16 rowOffset = ppu.scanline - (sprites[i].yPos & 0x7);

            if(sprites[i].attributes & SpriteVerticalFlip) { // Flipped vertically
                // TODO clean
                spriteAddr += 7 - rowOffset;

                if(ppu.scanline - sprites[i].yPos < 8) {
                    // read top half
                    // TODO clean
                } else {
                    // read bottom half
                    // next tile
                    spriteAddr += 16;
                }
            } else { // Not flipped vertically
                spriteAddr += rowOffset;

                if(ppu.scanline - sprites[i].yPos < 8) {
                    // read top half
                    // TODO clean
                } else {
                    // read bottom half
                    // next tile
                    spriteAddr += 16;
                }
            }

        } else { // 8x8

            u16 addressOffset = ppu.controllerReq & SpritePatterntableAddress ?  0x1000 : 0;
            spriteAddr = sprites[i].tileIndex * 16 + addressOffset;

            i32 rowOffset = (i32)ppu.scanline - (i32)sprites[i].yPos;

            //if(sprites[i].tileIndex == 0xDD) {
            //    LOG("x sprites[i].x %d %d", (int)sprites[i].xPos, (int)ppu.oam.xCounters[i]);
            //}

            if(sprites[i].attributes & SpriteVerticalFlip) {
                // Flipped vertically
                spriteAddr += 7 - rowOffset;
            } else {
                // Not flipped vertically
                spriteAddr += rowOffset;
            }
        }

        u8 lowerPatternByte = ppu_read(spriteAddr);
        u8 higherPatternByte = ppu_read(spriteAddr + 8);

        // Do horizontal flipping
        if(sprites[i].attributes & SpriteHorizontalFlip) {

            static unsigned char lookup[16] = {
                0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
                0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
            };

            // Reverse the top and bottom nibble then swap them.
            lowerPatternByte = (lookup[lowerPatternByte&0b1111] << 4) | lookup[lowerPatternByte>>4];
            higherPatternByte = (lookup[higherPatternByte&0b1111] << 4) | lookup[higherPatternByte>>4];
        }

        ppu.spriteLowShifter[i] = lowerPatternByte;
        ppu.spriteHighShifter[i] = higherPatternByte;
    }
}

static inline void
increment_coarseX() {
    if(ppu.loopyV.coarseX == 31) { // wrap around to next table
        ppu.loopyV.coarseX = 0;
        ppu.loopyV.nametableSelect ^= 0x1; //change nametable X
    } else {
        ppu.loopyV.coarseX += 1;
    }
}

static void
ppu_clock() {

    // TODO reorder
    if(ppu.scanline >= -1 && ppu.scanline < 240) { // out of vertical blank

        if (ppu.scanline == 0 && ppu.cycle == 0)
        {
            // "Odd Frame" cycle skip
            ppu.cycle = 1;
        }

        if(ppu.scanline == -1 && ppu.cycle == 1) {
            ppu.statusReq = 0;
        }
        /*between visible range or load next frames first */
        if((ppu.cycle >= 2 && ppu.cycle < 258) || (ppu.cycle >= 321 && ppu.cycle < 338)) {

            // update shifters
            ppu.shifterLow <<= 1;
            ppu.shifterHigh <<= 1;
            // TODO attribute

            ppu.paletteShifterLow <<= 1;
            ppu.paletteShifterHigh <<= 1;

            switch((ppu.cycle - 1) % 8) {
                case 0: // NT byte
                    {
                        load_shifters(); // update last loaded values to shifters

                        u16 patternIndex =  PPU_NAMETABLE_MEMORY_START + (ppu.loopyV.reqister & 0x0FFF);
                        ppu.NTbyte = ppu_read(patternIndex); // TODO check
                    } break;
                case 2: // AT byte
                    {
                        u16 attributeIndex = ((ppu.loopyV.coarseY >> 2) << 3)// divide by 4
                            | (ppu.loopyV.coarseX >> 2)                     // divide by 4
                            | (ppu.loopyV.nametableSelect << 10);           //correct nametables (x and y)

                        attributeIndex += 0x23C0; // starts at 23C0

                        u8 tempPalettes = ppu_read(attributeIndex);// TODO check

                        // determine which palette of 4 is used
                        if(ppu.loopyV.coarseY & 0x2) tempPalettes >>= 4;
                        if(ppu.loopyV.coarseX & 0x2) tempPalettes >>= 2;

                        ppu.ATbyte = tempPalettes & 0x3;

                    } break;
                case 4: // Low BG byte
                    {
                        // which table
                        u16 address = ppu.controllerReq & BackgroundTableAddress ? 0x1000 : 0x0;
                        // TODO check
                        address += ppu.NTbyte * 16; // tile is multiplied by size of tile
                        address += ppu.loopyV.fineY; // 0 - 7 to height (smooth scrolling)
                        ppu.LowBGbyte = ppu_read(address);
                    } break;
                case 6: // High BG byte
                    {
                        // which table
                        u16 address = ppu.controllerReq & BackgroundTableAddress ? 0x1000 : 0x0;
                        address += ppu.NTbyte * 16; // tile is multiplied by size of tile
                        address += ppu.loopyV.fineY; // 0 - 7 to height (smooth scrolling)
                        ppu.HighBGbyte = ppu_read(address + 8 /*high byte*/);
                    } break;
                case 7: // inc hori V
                    {
                        if(ppu.maskReq & (ShowBackground | ShowSprites)) { //TODO check place
                            increment_coarseX();
                        }
                    } break;
            }
        }

        if(ppu.cycle == 256) {
            // increment cource Y
            if(ppu.maskReq & (ShowBackground | ShowSprites)) { //TODO check place
                if(ppu.loopyV.fineY < 7) {
                    ppu.loopyV.fineY += 1;
                } else {
                    // TODO ???
                    //ASSERT_MESSAGE((u16)ppu.loopyV.coarseY < 30u,
                    //        "coarse Y overflow! %d", (u16)ppu.loopyV.coarseY);


                    ppu.loopyV.fineY = 0;

                    if(ppu.loopyV.coarseY == 29) { // wrap around
                        ppu.loopyV.coarseY = 0;
                        ppu.loopyV.nametableSelect ^= 0x2; //change nametable y
                    } else if (ppu.loopyV.coarseY == 31){
                        ppu.loopyV.coarseY = 0;
                    } else {
                        ppu.loopyV.coarseY += 1;
                    }
                }
            }
        }

        if(ppu.cycle == 257) { // hori (v) = hori (t)
            // Fetch next scanline sprites TODO
            ppu_oam_fetch_sprites();

            // load shifters
            load_shifters();
            // reset X
            if(ppu.maskReq & (ShowBackground | ShowSprites)) { //TODO check place
                ppu.loopyV.nametableSelect = (ppu.loopyV.nametableSelect & 0x2) | (ppu.loopyT.nametableSelect & 0x1); // set nametable X
                ppu.loopyV.coarseX = ppu.loopyT.coarseX;
            }
        }

        if(ppu.cycle == 340) {
            ppu_load_sprite_shifters();
        }

        // TODO remove and test
        if(ppu.cycle == 338 || ppu.cycle == 340) {
            ppu.NTbyte = ppu_read(PPU_NAMETABLE_MEMORY_START + (ppu.loopyV.reqister & 0x0FFF));
        }


        if(ppu.scanline == -1 && ppu.cycle >= 280 && ppu.cycle >= 304) {
            // Reset Y
            if(ppu.maskReq & (ShowBackground | ShowSprites)) { //TODO check place
                ppu.loopyV.nametableSelect = (ppu.loopyT.nametableSelect & 0x2) | (ppu.loopyV.nametableSelect & 0x1); // set nametable Y
                ppu.loopyV.coarseY = ppu.loopyT.coarseY;
                ppu.loopyV.fineY = ppu.loopyT.fineY;
            }
        }
    }

    // out of visible area
    if(ppu.scanline == 241 && ppu.cycle == 1) {
        ppu.statusReq |= VerticalBlankStarted;
        if(ppu.controllerReq & GenerateNMI) {
            ppu.NMIGenerated = 1;
        }
    }

    if((ppu.cycle - 1) >= 0 && (ppu.cycle - 1) < ((TEX_WIDTH)) &&
            ppu.scanline >= 0 && ppu.scanline < (TEX_HEIGHT)) {

        // render / put pixel
        u8 bgPixel = 0, bgPalette = 0;

        if(ppu.maskReq & ShowBackground) {

            u16 mux = 0x8000 >> ppu.fineX;

            uint8_t bit1 = (ppu.shifterLow & mux) > 0;
            uint8_t bit2 = (ppu.shifterHigh & mux) > 0;

            bgPixel = (bit2 << 1) | bit1;

            bit1 = (ppu.paletteShifterLow & mux) > 0;
            bit2 = (ppu.paletteShifterHigh & mux) > 0;

            bgPalette = (bit2 << 1) | bit1;
        }

        u8 fgPixel = 0, fgPalette = 0, fgPriority = 0;
#if 1
        u8 zeroIndexRendered = 0;

        if(ppu.maskReq & ShowSprites) {

            for(u8 i = 0; i < ppu.oam.numSpritesFound; i++) {

                i32 diff = (i32)(ppu.cycle - 1) - (i32)ppu.oam.xCounters[i]; // TODO migh be wrong

                if(diff >= 0 && diff < 8) {

                    u8 fgPixelLow = (ppu.spriteLowShifter[i] & 0x80) > 0;
                    u8 fgPixelHigh = (ppu.spriteHighShifter[i] & 0x80) > 0;

                    fgPixel = (fgPixelHigh << 1) | fgPixelLow;

                    fgPalette = (ppu.oam.attributeLatches[i] &
                            (SpritePaletteLow | SpritePaletteHigh)) + 4;

                    fgPriority = (ppu.oam.attributeLatches[i] & SpritePriority) == 0;

                    ppu.spriteLowShifter[i] <<= 1;
                    ppu.spriteHighShifter[i] <<= 1;

                    // Render the pixel
                    if(fgPixel != 0) {
                        zeroIndexRendered = i == 0;
                        break;
                    }

                }
            }
        }
#endif

        u8 finalPixel = 0, finalPalette = 0;

        // determine if you render background or sprite
        if(bgPixel == 0 && fgPixel == 0) {
            // Do nothing
        } else if (bgPixel == 0 && fgPixel != 0) {
            // Draw foregroung
            finalPixel = fgPixel;
            finalPalette = fgPalette;
        } else if (bgPixel != 0 && fgPixel == 0) {
            // Draw background
            finalPixel = bgPixel;
            finalPalette = bgPalette;
        } else {

            if(fgPriority) {
                finalPixel = fgPixel;
                finalPalette = fgPalette;
            } else {

                finalPixel = bgPixel;
                finalPalette = bgPalette;
            }

            if(zeroIndexRendered && ppu.oam.spriteZeroRendered && (ppu.maskReq & ShowBackground)) {
                // Sprite zero hit
                u8 xPos = 0;
                // http://wiki.nesdev.com/w/index.php/PPU_OAM#Sprite_zero_hits
                if(ppu.maskReq & SpriteIn8MostLeft || ppu.maskReq & BackgroungIn8MostLeft) {
                    xPos = 8;
                }

                if(ppu.cycle > xPos && ppu.cycle < 258 && ppu.cycle != 255) {
                    ppu.statusReq |= Sprite0Hit;
                }
            }
        }



        Color color = ppu_palette_get_color(finalPixel, finalPalette);
        memcpy(ppu.screen.data +
                (ppu.scanline * TEX_WIDTH + (ppu.cycle - 1)) * sizeof(Color),
                &color, sizeof(Color));
    }

    // update cycle
    ppu.cycle++;

    if (ppu.cycle >= 341)
    {
        ppu.cycle = 0;
        ppu.scanline++;
        if (ppu.scanline >= 261) // TODO wrong??
        {
            ppu.scanline = -1;
            ppu.frameComplete = 1;
        }
    }
}

static void
ppu_cpu_write(u16 addr, u8 data) {

    ASSERT_MESSAGE( (addr >= 0x2000 && addr <= 0x2007) || addr == PPU_DMA_WRITE_ADDRESS,
            "Incorrect ppu write 0x%04X", addr);

    // https://wiki.nesdev.com/w/index.php/PPU_registers#OAMDMA
    if(addr == PPU_DMA_WRITE_ADDRESS) {
        ppu.oam.DMAactive = DMAWaitingForCopy;
        ppu.oam.DMAaddr = data;
    }

    addr &= 0x7;

    switch(addr) {
        case 0x0: //PPUCTRL
            {
                ppu.controllerReq = data;
                ppu.loopyT.nametableSelect = ppu.controllerReq & 0x3;
                // TODO should generate NMI if in vertical blank?
            } break;
        case 0x1: //PPUMASK
            {
                ppu.maskReq = data;
            } break;
        case 0x2: //PPUSTATUS
            {

            } break;
        case 0x3: //OAMADDR
            {
                ppu.oam.addr = data;
            } break;
        case 0x4: //OAMDATA
            {
                ppu.oam.primary[ppu.oam.addr] = data;
                // https://wiki.nesdev.com/w/index.php/PPU_registers#OAMDATA
                ppu.oam.addr += 1; // TODO check increment
            } break;
        case 0x5: //PPUSCROLL
            {
                if(ppu.dataAddrAccess == 0) {
                    //2005 first write:
                    //t:0000000000011111=d:11111000
                    //x=d:00000111

                    ppu.fineX = data & 0x07; //(8 sprite lenght) TODO assert ?
                    ppu.loopyT.coarseX = data >> 3;
                } else {
                    //2005 second write:
                    //t:0000001111100000=d:11111000
                    //t:0111000000000000=d:00000111
                    ppu.loopyT.fineY = data & 0x07;
                    ppu.loopyT.coarseY = data >> 3;
                }

                ppu.dataAddrAccess ^= 0x1;
            } break;
        case 0x6: //PPUADDR
            {
                ppu.loopyT.reqister = ppu.dataAddrAccess ?
                    (ppu.loopyT.reqister & 0xFF00) | data :
                    (ppu.loopyT.reqister & 0x00FF) | (data << 8);

                // if full address range written update vram address
                if(ppu.dataAddrAccess) ppu.loopyV = ppu.loopyT;

                ppu.dataAddrAccess ^= 0x1;
            } break;
        case 0x7: //PPUDATA
            {
                ppu_write(ppu.loopyV.reqister, data);
                ppu.loopyV.reqister += ppu.controllerReq & VramAddressIncrement ? 32 : 1;
            } break;
        default:
            {
                ABORT("unknown ppu write");
            } break;
    }
}

static u8
ppu_cpu_read(u16 addr) {
    addr &= 0x7;
    u8 ret = 0;
    switch(addr) {
        case 0x2: //PPUSTATUS
            {
                ret = ppu.statusReq;
                ppu.statusReq &= ~VerticalBlankStarted;
                //ppu.dataAddr = 0; //TODO
            } break;
        case 0x4: {
                      ret = ppu.oam.primary[ppu.oam.addr];
                  } break;
        case 0x7: //PPUDATA
                  {
                      if(address_is_between(ppu.loopyV.reqister,
                                  PPU_PALETTE_MEMORY_START, PPU_PALETTE_MEMORY_END)) {
                          // The palette data is placed immediately on the data bus,
                          // and hence no dummy read is required.
                          // Reading the palettes still updates the internal buffer though,
                          // but the data placed in it is the mirrored nametable data that would appear
                          // "underneath" the palette.

                          ret = ppu.internalDataBuffer = ppu_read(ppu.loopyV.reqister);
                      } else {
                          // When reading while the VRAM address is in the range 0-$3EFF
                          // (i.e., before the palettes),
                          // the read will return the contents of an internal read buffer.
                          //
                          // After the CPU reads and gets the contents of the internal buffer,
                          // the PPU will immediately update the internal buffer with the byte at the
                          // current VRAM address.
                          // Thus, after setting the VRAM address,
                          // one should first read this register and discard the result.
                          ret = ppu.internalDataBuffer;
                          ppu.internalDataBuffer = ppu_read(ppu.loopyV.reqister);
                      }

                      ppu.loopyV.reqister += ppu.controllerReq & VramAddressIncrement ? 32 : 1;

                  } break;
        default:
                  {
                      //ABORT("error ppu read! 0x%04X", addr);
                  }
    }
    return ret;
}

static void
ppu_render_patterntable(u8 index, u32 paletteIndex) { // there is 2 pattern tables so this is 0 or 1
#ifndef LOGFILE
    for(u16 tileY = 0; tileY < NUM_TILES; tileY++) { // FOR TILE Y

        for(u16 tileX = 0; tileX < NUM_TILES; tileX++) { // FOR TILE X

            u16 tileoffset = (tileY * NUM_TILES + tileX)
                * 16 // one tile size
                + index * 0x1000; // which patterntable

            for(u16 pixelY = 0; pixelY < TILE_DIM; pixelY++) { // FOR PIXEL Y


                u8 lsb = ppu_read(tileoffset + pixelY);
                u8 msb = ppu_read(tileoffset + 8 + pixelY);

                u16 imageY = tileY * TILE_DIM + pixelY;

                for(u16 pixelX = 0; pixelX < TILE_DIM; pixelX++) {  // FOR PIXEL X

                    u16 imageX = (tileX * TILE_DIM + (7 - pixelX));

                    u8 pixel = ((msb & 0x1) << 1) | (lsb & 0x1);

                    lsb >>= 1;
                    msb >>= 1;

                    // Set pixel in texture
                    Color color = ppu_palette_get_color(pixel, paletteIndex);

                    // Draw the pixel
                    memcpy(ppu.pattern[index].data +
                            (imageY * ppu.pattern[index].w + imageX) * sizeof(Color),
                            &color,
                            sizeof(Color));
                }
            }
        }
    }
#endif
}

static void
ppu_render_oam() { // there is 2 pattern tables so this is 0 or 1
#if 1

    memset(ppu.OAMvisualisation.data, 0,
            ppu.OAMvisualisation.w * ppu.OAMvisualisation.h * sizeof(Color));

    OAMData* data = (OAMData*)ppu.oam.primary; // TODO fix pointer cast
    for(u16 tileY = 0; tileY < 8; tileY++) { // FOR TILE Y

        for(u16 tileX = 0; tileX < 8; tileX++) { // FOR TILE X

            //if(data[(tileY * 8) + tileX].yPos >= 240 /*|| data[(tileY * 8) + tileX].attributes == 0*/) {
            //    continue;
            //}

            u8 palette = (data[(tileY * 8) + tileX].attributes &
                    (SpritePaletteLow | SpritePaletteHigh)) + 4;

            u16 tileoffset = data[(tileY * 8) + tileX].tileIndex * 16;

            for(u16 pixelY = 0; pixelY < TILE_DIM; pixelY++) { // FOR PIXEL Y

                u8 lsb = ppu_read(tileoffset + pixelY);
                u8 msb = ppu_read(tileoffset + 8 + pixelY);

                u16 imageY = tileY * TILE_DIM + pixelY;

                for(u16 pixelX = 0; pixelX < TILE_DIM; pixelX++) {  // FOR PIXEL X

                    u16 imageX = (tileX * TILE_DIM + (7 - pixelX));

                    u8 pixel = ((msb & 0x1) << 1) | (lsb & 0x1);

                    lsb >>= 1;
                    msb >>= 1;

                    // Set pixel in texture
                    Color color = ppu_palette_get_color(pixel, palette);
#if 1
                    // Draw the pixel
                    memcpy(ppu.OAMvisualisation.data +
                            (imageY * ppu.OAMvisualisation.w + imageX) * sizeof(Color),
                            &color,
                            sizeof(Color));
#endif
                }
            }
        }
    }
#endif
}

static ImageView
imageview_create(u32 w, u32 h) {

    ImageView ret = { .w = w, .h = h, .data = calloc(w * h, sizeof(Color))};
    GLCHECK(glGenTextures(1, &ret.tex));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, ret.tex));

    GLCHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, ret.data));

    GLCHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));

    return ret;
}

static void
imageview_update(ImageView* view) {

    GLCHECK(glBindTexture(GL_TEXTURE_2D, view->tex));
    GLCHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view->w, view->h,
                GL_RGBA, GL_UNSIGNED_BYTE, view->data));
    GLCHECK(glBindTexture( GL_TEXTURE_2D, 0));
}




u32
shader_compile(GLenum type, const char* source) {
    i32 compiledcheck;

    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        LOG("Failed to create shader");
    }

    GLCHECK(glShaderSource(shader, 1, &source, NULL));
    GLCHECK(glCompileShader(shader));

    GLCHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &compiledcheck));
    if (!compiledcheck)
    {
        i32 infoLen = 0;
        GLCHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen));
        if (infoLen > 1)
        {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            GLCHECK(glGetShaderInfoLog(shader, infoLen, NULL, infoLog));
            LOG("Error compiling shader :\n%s", infoLog);
            free(infoLog);
        }
        GLCHECK(glDeleteShader(shader));
        exit(1);
    }
    gl_check_error();
    return shader;
}


float vertices[] = {
    // positions        // texture coords
    0.5f,  0.5f, 1.0f, 1.0f,   // top right
    0.5f, -0.5f, 1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f, 1.0f    // top left
};

unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

static void
ppu_init() {

    memset(&ppu, 0, sizeof(struct PPU));

    //memset(ppu.screen.data, 200, ppu.screen.w * ppu.screen.h * (sizeof(u8) * 3));
    size_t size = 0;
    char* vs = load_file("vert.sha", &size);
    if(!vs) {
        LOG("failed to load vert.sha");
        exit(EXIT_FAILURE);
    }
    LOG("%s", vs);

    GLuint vert = shader_compile(GL_VERTEX_SHADER, vs);
    free(vs);

    char* fs = load_file("frag.sha", &size);
    if(!fs) {
        LOG("failed to load vert.sha");
        exit(EXIT_FAILURE);
    }
    LOG("%s", fs);

    GLuint frag = shader_compile(GL_FRAGMENT_SHADER, fs);
    free(fs);
    ppu.renderProgram = glCreateProgram();

    GLCHECK(glAttachShader(ppu.renderProgram, vert));
    GLCHECK(glAttachShader(ppu.renderProgram, frag));

    GLCHECK(glBindAttribLocation(ppu.renderProgram, 0, "vertexPosition"));
    GLCHECK(glLinkProgram(ppu.renderProgram));


    ppu.transformLoc = glGetUniformLocation(ppu.renderProgram, "transform");
    if(ppu.transformLoc == -1) {
        LOG("didnt find transform location");
        exit(1);
    }

    ppu.projectionLoc = glGetUniformLocation(ppu.renderProgram, "projection");
    if(ppu.projectionLoc == -1) {
        LOG("didnt find projection location");
        exit(1);
    }


    // Create texture
    ppu.screen = imageview_create(TEX_WIDTH, TEX_HEIGHT);
    ppu.pattern[0] = imageview_create(TILE_DIM * NUM_TILES, TILE_DIM * NUM_TILES);
    ppu.pattern[1] = imageview_create(TILE_DIM * NUM_TILES, TILE_DIM * NUM_TILES);
    ppu.OAMvisualisation = imageview_create(8 * TILE_DIM , 8 * TILE_DIM);


    // generte vertex data
    GLCHECK(glGenVertexArrays(1, &ppu.vao));
    GLCHECK(glBindVertexArray(ppu.vao));

    u32 EBO;
    glGenBuffers(1, &EBO);
    GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GLCHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));


    u32 vertbuff;
    GLCHECK(glGenBuffers(1, &vertbuff));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, vertbuff));
    GLCHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));



    // position attribute
    GLCHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
    GLCHECK(glEnableVertexAttribArray(0));

    // uv attribute
    GLCHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(float), (void*)(2 * sizeof(float))));
    GLCHECK(glEnableVertexAttribArray(1));

    GLCHECK(glBindVertexArray(0));

    GLCHECK(glEnable(GL_DEPTH_TEST));
    GLCHECK(glDepthMask(GL_FALSE));
}

static void
ppu_render() {

    imageview_update(&ppu.screen);
}
#endif /* PPU_H */
