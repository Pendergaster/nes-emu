/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

static u8 ppu_read(u16 addr);

#ifndef PPU_H
#define PPU_H

#define NAMETABLE_SIZE                  0x400 // 1024
#define TEX_HEIGHT                      256
#define TEX_WIDTH                       240

#define TILE_DIM                        8
#define NUM_TILES                       16

#define PPU_MAX_MEMORY_ADDR             0x3FFF

#define PPU_PATTERN_MEMORY_START        0x0
#define PPU_PATTERN_MEMORY_END          0x1FFF

#define PPU_NAMETABLE_MEMORY_START      0x2000
#define PPU_NAMETABLE_MEMORY_END        0x3EFF

#define PPU_PALETTE_MEMORY_START        0x3F00
#define PPU_PALETTE_MEMORY_END          0x3FFF

#include "cmath.h"

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

STATIC_ASSERT(sizeof(Loopy) == sizeof(u16), loopy_size_wrong);

struct PPU {
    u8          nameTables[2 * NAMETABLE_SIZE]; // layout of background 0x2000 - 0x3F00
    u8          patternTables[2][4096];         // sprites 0x0 - 0x1FFF
    u8          palette[32];                    // color palette 0x3F00 - 0x3FFF

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
    //u16         dataAddr; // PPUADDR / PPUDATA

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
    u16         paletteShifterHigh; // TODO rework
    u16         paletteShifterLow;  // TODO rework

    u8          NMIGenerated; // TODO sould generate one in cpu write?
    // http://wiki.nesdev.com/w/index.php/PPU_registers#PPUCTRL

    ImageView   screen;
    ImageView   pattern[2];

    GLuint      renderProgram;
    GLint       transformLoc;
    GLint       projectionLoc;
    GLuint      vao;
} ppu;

typedef struct Color {
    u8 r,g,b;
} Color;

Color colors[0x40] = {
    {84, 84, 84}, {0, 30, 116}, {8, 16, 144}, {48, 0, 136}, {68, 0, 100}, {92, 0, 48}, {84, 4, 0},
    {60, 24, 0}, {32, 42, 0}, {8, 58, 0}, {0, 64, 0}, {0, 60, 0}, {0, 50, 60}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {152, 150, 152}, {8, 76, 196}, {48, 50, 236}, {92, 30, 228}, {136, 20, 176},
    {160, 20, 100}, {152, 34, 32}, {120, 60, 0}, {84, 90, 0}, {40, 114, 0}, {8, 124, 0}, {0, 118, 40},
    {0, 102, 120}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {236, 238, 236}, {76, 154, 236}, {120, 124, 236},
    {176, 98, 236}, {228, 84, 236}, {236, 88, 180}, {236, 106, 100}, {212, 136, 32}, {160, 170, 0},
    {116, 196, 0}, {76, 208, 32}, {56, 204, 108}, {56, 180, 204}, {60, 60, 60}, {0, 0, 0}, {0, 0, 0},
    {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236},
    {236, 174, 212}, {236, 180, 176}, 228, 196, 144, {204, 210, 120}, {180, 222, 120},
    {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {0, 0, 0}, {0, 0, 0}
};


static u8
ppu_read(u16 addr) {

    u8 data = 0x00;
    addr &= PPU_MAX_MEMORY_ADDR;

    if(address_is_between(addr,
                PPU_PATTERN_MEMORY_START, PPU_PATTERN_MEMORY_END)) {

        i8 table = (addr & 0x1000) >> 12;
        i8 tableIndex = addr & 0x0FFF;

        data = ppu.patternTables[table][tableIndex];
        // TODO fix or sth
        data = cartridge_ppu_read_rom(addr);

    } else if (address_is_between(addr,
                PPU_NAMETABLE_MEMORY_START, PPU_NAMETABLE_MEMORY_END)) {

        switch(cartridge.mirrorType){
            case VERTICAL:
                {
                    //[0][1]
                    //[0][1]
                    u8 readAddr = addr % NAMETABLE_SIZE;

                    if(addr < NAMETABLE_SIZE) {
                        data = ppu.nameTables[readAddr];
                    } else if(addr < NAMETABLE_SIZE * 2) {
                        data = ppu.nameTables[NAMETABLE_SIZE + readAddr];
                    }else if(addr < NAMETABLE_SIZE * 3) {
                        data = ppu.nameTables[readAddr];
                    } else {
                        data = ppu.nameTables[NAMETABLE_SIZE + readAddr];
                    }
                } break;
            case HORIZONTAL:
                {
                    //[0][0]
                    //[1][1]
                    u8 readAddr = addr % NAMETABLE_SIZE;
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

        data = ppu.palette[addr];

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


        i8 table = (addr & 0x1000) >> 12;
        i8 tableIndex = addr & 0x0FFF;

        ppu.patternTables[table][tableIndex] = data;

    } else if (address_is_between(addr,
                PPU_NAMETABLE_MEMORY_START, PPU_NAMETABLE_MEMORY_END)) {


        switch(cartridge.mirrorType){
            case VERTICAL:
                {
                    //[0][1]
                    //[0][1]
                    u8 readAddr = addr % NAMETABLE_SIZE;

                    LOG("writing to read addr! %d", readAddr);

                    if(addr < NAMETABLE_SIZE) {
                        ppu.nameTables[readAddr] = data ;
                    } else if(addr < NAMETABLE_SIZE * 2) {
                        ppu.nameTables[NAMETABLE_SIZE + readAddr] = data;
                    }else if(addr < NAMETABLE_SIZE * 3) {
                        ppu.nameTables[readAddr] = data;
                    } else {
                        ppu.nameTables[NAMETABLE_SIZE + readAddr] = data;
                    }
                } break;
            case HORIZONTAL:
                {

                    //[0][0]
                    //[1][1]
                    u8 readAddr = addr % NAMETABLE_SIZE;

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
ppu_clock() {

    CHECKLOG;

#ifdef LOG
    fprintf(logfile, "scanline %d cycle %d status %d mask %d control %d loopyT %d looyV %d\n",
            ppu.scanline, ppu.cycle, ppu.statusReq, ppu.maskReq, ppu.controllerReq, ppu.loopyT.reqister,
            ppu.loopyV.reqister);
#endif

#if 1

    // TODO reorder
    if(ppu.scanline >= -1 && ppu.scanline < 240) { // out of vertical blank

        if (ppu.scanline == 0 && ppu.cycle == 0)
        {
            // "Odd Frame" cycle skip
            ppu.cycle = 1;
        }

        if(ppu.scanline == -1 && ppu.cycle == 1) {
            ppu.statusReq &= ~VerticalBlankStarted;
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
                            if(ppu.loopyV.coarseX == 31) { // wrap around to next table
                                ppu.loopyV.coarseX = 0;
                                ppu.loopyV.nametableSelect ^= 0x1; //change nametable X
                            } else {
                                ppu.loopyV.coarseX += 1;
                            }
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
                    ASSERT_MESSAGE((u16)ppu.loopyV.coarseY < 30u,
                            "coarse Y overflow! %d", (u16)ppu.loopyV.coarseY);

                    if(ppu.loopyV.coarseY == 29) { // wrap around
                        ppu.loopyV.coarseY = 0;
                        ppu.loopyV.nametableSelect ^= 0x2; //change nametable y
                    } else {
                        ppu.loopyV.coarseY += 1;
                    }
                }
            }
        }

        if(ppu.cycle == 257) { // hori (v) = hori (t)
            // load shifters
            load_shifters();
            // reset X
            if(ppu.maskReq & (ShowBackground | ShowSprites)) { //TODO check place
                ppu.loopyV.nametableSelect |= (ppu.loopyT.nametableSelect & 0x1); // set nametable X
                ppu.loopyV.coarseX = ppu.loopyT.coarseX;
            }
        }

        // TODO remove and test
        if(ppu.cycle == 338 || ppu.cycle == 340) {
            ppu.NTbyte = ppu_read(PPU_NAMETABLE_MEMORY_START + (ppu.loopyV.reqister & 0x0FFF));
        }


        if(ppu.scanline == -1 && ppu.cycle >= 280 && ppu.cycle >= 304) {
            // Reset Y
            if(ppu.maskReq & (ShowBackground | ShowSprites)) { //TODO check place
                ppu.loopyV.nametableSelect |= (ppu.loopyT.nametableSelect & 0x2); // set nametable Y
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

    if((ppu.cycle - 1) >= 0 && (ppu.cycle - 1) < TEX_WIDTH &&
            ppu.scanline >= 0 && ppu.scanline < TEX_HEIGHT) {


        // render / put pixel
        u8 pixel = 0;
        u8 palette = 0;
        if(ppu.maskReq & ShowBackground) {

            u16 mux = 0x8000 >> ppu.fineX;

            uint8_t bit1 = (ppu.shifterLow & mux) > 0;
            uint8_t bit2 = (ppu.shifterHigh & mux) > 0;

            pixel = (bit2 << 1) | bit1;

            bit1 = (ppu.paletteShifterLow & mux) > 0;
            bit2 = (ppu.paletteShifterHigh & mux) > 0;

            palette = (bit2 << 1) | bit1;
        }

        Color color = ppu_palette_get_color(pixel, palette);

        //LOG("y %d x %d", ppu.scanline, ppu.cycle - 1);

        memcpy(&ppu.screen.data[(ppu.scanline * TEX_WIDTH + (ppu.cycle - 1)) * 3] ,
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



#else
    ppu.cycle++;

    u8 color[3] = {rand() & 0xFF, rand() & 0xFF, rand() & 0xFF};

    int x = rand() % (TEX_WIDTH);
    int y = rand() % (TEX_HEIGHT);

    memcpy(&ppu.screen.data[(y * TEX_WIDTH + x) * 3] ,color, sizeof(u8) * 3);

    if(ppu.cycle > TEX_WIDTH) { // reset
        ppu.cycle = 0;
        ppu.scanline += 1;

        if(ppu.scanline > TEX_HEIGHT) { // reset
            ppu.scanline = -1;
            ppu.frameComplete = 1;
        }
    }
#endif
}

static void
ppu_cpu_write(u16 addr, u8 data) {

    ASSERT_MESSAGE( (addr >= 0x2000 && addr <= 0x2007) || addr == 0x4014,
            "Incorrect ppu write 0x%04X", addr);

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

            } break;
        case 0x4: //OAMDATA
            {

            } break;
        case 0x5: //PPUSCROLL
            {
                if(ppu.dataAddrAccess) {
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

                    u8 pixel = (lsb & 0x1) + (msb & 0x1);

                    lsb >>= 1;
                    msb >>= 1;

                    // Set pixel in texture
                    Color color = ppu_palette_get_color(pixel, paletteIndex);

                    // Draw the pixel
                    memcpy(ppu.pattern[index].data + (imageY * ppu.pattern[index].w + imageX) * sizeof(Color),
                            &color, sizeof(Color));
                }
            }
        }
    }
}

static ImageView
imageview_create(u32 w, u32 h) {

    ImageView ret = { .w = w, .h = h, .data = calloc(w * h, sizeof(Color))};
    GLCHECK(glGenTextures(1, &ret.tex));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, ret.tex));

    GLCHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
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
                GL_RGB, GL_UNSIGNED_BYTE, view->data));
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

    memset(ppu.screen.data, 200, ppu.screen.w * ppu.screen.h * sizeof(Color));
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
