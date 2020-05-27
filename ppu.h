/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */


static u8 ppu_read(u16 addr);


#ifndef PPU_H
#define PPU_H

#define NAMETABLE_SIZE  1024
#define TEX_HEIGHT      256
#define TEX_WIDTH       240

#define TILE_DIM        8
#define NUM_TILES       16

#define PPU_MAX_MEMORY_ADDR 0x3FFF

#define PPU_PATTERN_MEMORY_START        0x0
#define PPU_PATTERN_MEMORY_END          0x1FFF

#define PPU_NAMETABLE_MEMORY_START      0x2000
#define PPU_NAMETABLE_MEMORY_END        0x3EFF

#define PPU_PALETTE_MEMORY_START        0x3F00
#define PPU_PALETTE_MEMORY_END          0x3FFF

#include "cmath.h"

static FILE* file;
static int count;

typedef struct ImageView {
    u8*     data;
    u32     w,h;
    GLuint  tex;
} ImageView;


// http://wiki.nesdev.com/w/index.php/PPU_registers
typedef enum PPUStatus {
    SpriteOverflow          = (1 << 5),
    Sprite0Hit              = (1 << 6),
    VerticalBlankStarted    = (1 << 7),
} PPUStatus ;

typedef enum PPUMask {
    GreyScale                   = (1 << 0),
    BackgroungIn8MostLeft       = (1 << 1),
    SpriteIn8MostLeft           = (1 << 2),
    ShowBackground              = (1 << 3),
    ShowSprites                 = (1 << 4),
    EmphasizeRed                = (1 << 5),
    EmphasizeGreen              = (1 << 6),
    EmphasizeBlue               = (1 << 7)
} PPUMask ;

typedef enum PPUController {
    BaseNameTableAddress1       = (1 << 0),
    BaseNameTableAddress2       = (1 << 1),
    VramAddressIncrement        = (1 << 2), // (0: add 1, going across; 1: add 32, going down)
    SpritePatterntableAddress   = (1 << 3),
    BackgroundTableAddress      = (1 << 4),
    SpriteSize                  = (1 << 5), // Sprite size (0: 8x8 pixels; 1: 8x16 pixels)
    PPUMasterSlaveSelect        = (1 << 6), // 0: read backdrop from EXT pins;
    // 1: output color on EXT pins
    GenerateNMI                 = (1 << 7)
} PPUController ;


struct PPU {
    u8          nameTables[2 * NAMETABLE_SIZE]; // layout of background 0x2000 - 0x3F00
    u8          patternTables[2][4096];         // sprites 0x0 - 0x1FFF
    u8          palette[32];                    // layout of background 0x3F00 - 0x3FFF

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
    u16         dataAddr; // PPUADDR / PPUDATA

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


    } else if (address_is_between(addr,
                PPU_PALETTE_MEMORY_START, PPU_PALETTE_MEMORY_END)) {

        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004; // TODO check
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;

        data = ppu.palette[addr];

#ifdef COUNT
        if(count >= 0xC) {
            fprintf(file, "addr 0x%04X data 0x%04X\n", addr, data);
        }
#endif

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

    } else if (address_is_between(addr,
                PPU_PALETTE_MEMORY_START, PPU_PALETTE_MEMORY_END)) {

        u16 orgAdd = addr;
        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004; // TODO check
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;

        LOG("writing to 0x%04X %d (org 0x%04X)", addr, data, orgAdd);
#ifdef COUNT
        if(++count >= 0xC && !file) {
            LOG("Opened file!!!");
            file = fopen("log.txt", "w");
            return;
        }
#endif

        if(++count >= 0xC + 3) {
            exit(1);
            return;
        }

        ppu.palette[addr] = data;
    } else {
        LOG("TODO ERROR");
    }
}

static void
ppu_clock() {

#if 1

    if(ppu.scanline == -1 && ppu.cycle == 1) {
        ppu.statusReq &= ~VerticalBlankStarted;
    }

    if(ppu.scanline == 241 && ppu.cycle == 1) {
        ppu.statusReq |= VerticalBlankStarted;
        if(ppu.controllerReq & GenerateNMI) {
            ppu.NMIGenerated = 1;
        }
    }
    ppu.cycle++;
    if (ppu.cycle >= 341)
    {
        ppu.cycle = 0;
        ppu.scanline++;
        if (ppu.scanline >= 261)
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

static inline Color
ppu_palette_get_color(u8 pixel, u32 paletteIndex) {

    u16 paletteLocation = PPU_PALETTE_MEMORY_START + (paletteIndex * 4) + pixel;
#ifdef COUNT
    if(count >= 0xC) {
        fprintf(file,"location 0x%04X\n", paletteLocation );
    }
#endif
    u8 data = ppu_read(paletteLocation);

#ifdef COUNT
    //if(data >= SIZEOF_ARRAY(colors)) ABORT("Color mem overflow");
    Color RERERE = colors[data & 0x3F];
    if(count >= 0xC) {
        fprintf(file,"color %d %d %d\n", RERERE.r, RERERE.g, RERERE.b);
    }
#endif
    return colors[data & 0x3F];
}

static void
ppu_cpu_write(u16 addr, u8 data) {
    addr &= 0x7;

    switch(addr) {
        case 0x0: //PPUCTRL
            {
                ppu.controllerReq = data;
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

            } break;
        case 0x6: //PPUADDR
            {
                ppu.dataAddr = ppu.dataAddrAccess ?
                    (ppu.dataAddr & 0xFF00) | data :
                    (ppu.dataAddr & 0x00FF) | (data << 8);

                ppu.dataAddrAccess ^= 0x1;
            } break;
        case 0x7: //PPUDATA
            {
                ppu_write(ppu.dataAddr, data);
                ppu.dataAddr += ppu.controllerReq & VramAddressIncrement ? 32 : 1;
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
                ppu.dataAddr = 0;
            } break;
        case 0x7: //PPUDATA
            {
                if(address_is_between(ppu.dataAddr,
                            PPU_PALETTE_MEMORY_START, PPU_PALETTE_MEMORY_END)) {
                    // The palette data is placed immediately on the data bus,
                    // and hence no dummy read is required.
                    // Reading the palettes still updates the internal buffer though,
                    // but the data placed in it is the mirrored nametable data that would appear
                    // "underneath" the palette.

                    ret = ppu.internalDataBuffer = ppu_read(ppu.dataAddr);
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
                    ppu.internalDataBuffer = ppu_read(ppu.dataAddr);
                }

                ppu.dataAddr += ppu.controllerReq & VramAddressIncrement ? 32 : 1;

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
#ifdef COUNT
    if(count >= 0xC) {
        fprintf(file, "starting to read\n");
    }
#endif

    for(u16 tileY = 0; tileY < NUM_TILES; tileY++) { // FOR TILE Y
#ifdef COUNT
        if(count >= 0xC) {
            fprintf(file, "Ytile %d\n", tileY);
        }
#endif

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
#ifdef COUNT
    if(count >= 0xC) {
        fclose(file);
        exit(1);
    }
#endif
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
        printf("Failed to create shader\n");
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
            printf("Error compiling shader :\n%s\n", infoLog);
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

    memset(ppu.screen.data, 200, ppu.screen.w * ppu.screen.h * sizeof(Color));

    size_t size = 0;
    char* vs = load_file("vert.sha", &size);
    if(!vs) {
        printf("failed to load vert.sha\n");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", vs);

    GLuint vert = shader_compile(GL_VERTEX_SHADER, vs);
    free(vs);

    char* fs = load_file("frag.sha", &size);
    if(!fs) {
        printf("failed to load vert.sha\n");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", fs);

    GLuint frag = shader_compile(GL_FRAGMENT_SHADER, fs);
    free(fs);
    ppu.renderProgram = glCreateProgram();

    GLCHECK(glAttachShader(ppu.renderProgram, vert));
    GLCHECK(glAttachShader(ppu.renderProgram, frag));

    GLCHECK(glBindAttribLocation(ppu.renderProgram, 0, "vertexPosition"));
    GLCHECK(glLinkProgram(ppu.renderProgram));


    ppu.transformLoc = glGetUniformLocation(ppu.renderProgram, "transform");
    if(ppu.transformLoc == -1) {
        printf("didnt find transform location\n");
        exit(1);
    }

    ppu.projectionLoc = glGetUniformLocation(ppu.renderProgram, "projection");
    if(ppu.projectionLoc == -1) {
        printf("didnt find projection location\n");
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

#if 0
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    //glClearColor(1.f, 0.f, 1.f, 0.f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    // Draw all
    GLCHECK(glUseProgram(ppu.renderProgram));

    mat4 projection;

    identify_mat4(&projection);
#if 1
    orthomat(&projection, 0.0f,
            (float)SCREEN_WIDTH,
            (float)SCREEN_HEIGHT,
            0.0f, 0.1f, 100.f);
#endif

    GLCHECK(glUniformMatrix4fv(ppu.projectionLoc, 1, GL_FALSE, (float*)&projection));

    mat4 trans;
    create_translation_mat_inside(&trans,
            (vec3){ (float)TEX_WIDTH / 2.0 , (float)TEX_HEIGHT / 2.0, 1});

    mat4 scaling;
    identify_mat4(&scaling);
    create_scaling_mat4(&scaling, (vec3) {TEX_WIDTH, TEX_HEIGHT , 1});


    mat4 transform;
    identify_mat4(&transform);
    mat4_mult_mat4(&transform, &trans, &scaling);

    GLCHECK(glUniformMatrix4fv(ppu.transformLoc, 1, GL_FALSE, (float*)&transform));

    glBindTexture(GL_TEXTURE_2D, ppu.renderTexture);
    glBindVertexArray(ppu.vao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
#endif
}
#endif /* PPU_H */
