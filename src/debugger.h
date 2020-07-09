/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef CPUDEBUGGER_H
#define CPUDEBUGGER_H

#if 1
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#endif

#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

struct nk_image renderImage;
struct nk_image patternImage[2];
struct nk_image oamImage;


#include "style.c"
#include "overview.c"

/* GUI */
struct nk_context *ctx;
struct nk_colorf bg;


char** disassemblyTable;

static void
debugger_init(SDL_Window *win) {

    u32 programMemSize = cartridge.numProgramRoms * PROG_ROM_SINGLE_SIZE;

    disassemblyTable = calloc(programMemSize, sizeof(char *));

    // program memory start
    for(u32 i = mapper.programMemStart; i < (mapper.programMemStart + programMemSize); i++) {

        u16 index = i - mapper.programMemStart;
        char* disassembly = malloc(32);
        u8 opcode = bus_peak8(i);

        Instruction instruction = instructionTable[opcode];
        u16 low = 0x0;
        u16 high = 0x0;

        u32 instLen = strlen(cpuInstructionStrings[opcode]);
        memcpy(disassembly ,cpuInstructionStrings[opcode], instLen);

        disassembly[instLen] = ' ';

        if(instruction.addressMode == ABS ||                // fetch 2 addresses
                instruction.addressMode == ABSX ||
                instruction.addressMode == ABSY ||
                instruction.addressMode == IND
          ) {

            i++;
            low = bus_peak8(i);

            i++;
            high = bus_peak8(i);
        }
        else if( instruction.addressMode == IMP || // fetch 0 addresses
                instruction.addressMode == ACCUM //||
                //instruction.addressMode == IMM
               )
        {
            disassembly[instLen + 1] = 0;
            disassemblyTable[index] = disassembly;

            continue;
        }
        else // fetch low adress (1 address)
        {
            i++;
            low = bus_peak8(i); // TODO fix
        }

        u16 holeAddr = (high << 8) | low;
        char temp[32];

        sprintf(temp, "0x%04X", holeAddr);

        int hexStringSize = strlen(temp) + 1;
        memcpy(&disassembly[instLen + 1], temp, hexStringSize);

        disassemblyTable[index] = disassembly;
    }

    ctx = nk_sdl_init(win);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */

    {
        struct nk_font_atlas *atlas;
        nk_sdl_font_stash_begin(&atlas);
        struct nk_font *cousine =
            nk_font_atlas_add_from_file(atlas, "./fonts/extra_font/Cousine-Regular.ttf", 13, 0);

        nk_sdl_font_stash_end();
        nk_style_load_all_cursors(ctx, atlas->cursors);
        nk_style_set_font(ctx, &cousine->handle);
    }
    /* style.c */
    set_style(ctx, THEME_DARK);

    renderImage = nk_image_id(ppu.screen.tex);
    patternImage[0] = nk_image_id(ppu.pattern[0].tex);
    patternImage[1] = nk_image_id(ppu.pattern[1].tex);
    oamImage = nk_image_id(ppu.OAMvisualisation.tex);
}

static void
memory_debugger() {

    static u32 page;
    // Print reqisters
    nk_layout_row_static(ctx, 25, 100, 4);

    char reqString[64];
    sprintf ( reqString,"reqX 0x0%X", cpu.Xreq);
    nk_label(ctx, reqString, NK_TEXT_LEFT);
    sprintf ( reqString,"reqY 0x0%X", cpu.Yreq);
    nk_label(ctx, reqString, NK_TEXT_LEFT);
    sprintf ( reqString,"reqA 0x0%X", cpu.accumReq);
    nk_label(ctx, reqString, NK_TEXT_LEFT);
    sprintf ( reqString,"Stack 0x0%X", cpu.stackPointer);
    nk_label(ctx, reqString, NK_TEXT_LEFT);

    // Show flag status
    nk_layout_row_begin(ctx, NK_STATIC, 30, 9);
    nk_layout_row_push(ctx, 35);

    nk_label(ctx, "Flags", NK_TEXT_CENTERED);

    nk_layout_row_push(ctx, 50);
    char* flags[] = { "C", "Z","I", "D", "B" ,"U", "V", "N" };
    for(int i = 0; i < 8; i++){
        if(cpu.flags & (1 << i)) {
            nk_label_colored(ctx, flags[i], NK_TEXT_CENTERED, nk_rgb(200, 200, 0));
        } else {
            nk_label(ctx, flags[i], NK_TEXT_CENTERED);
        }
    }
    nk_layout_row_end(ctx);

    nk_layout_row_static(ctx, 25, 30, 0x11);
    nk_label(ctx, "", NK_TEXT_LEFT);
    // show 0x0F adresses
    char hexString[8];
    for(u8 i= 0; i < 0x10; i++) {
        sprintf ( hexString,"0x0%X", i);
        nk_label_colored(ctx, hexString, NK_TEXT_LEFT, nk_rgb(200, 200, 0));
    }

    for(int i= 0; i <= 0xFF; i++) {
        if((i & 0xF) == 0) {
            sprintf ( hexString,"0x%X0", (i >> 4));
            nk_label_colored(ctx, hexString, NK_TEXT_LEFT, nk_rgb(200, 200, 0));
            //nk_label(ctx, "AA", NK_TEXT_LEFT);
        }
        i32 selected = (int)page == i;

        sprintf ( hexString,"0x%X", (bus_peak8( (page << 8) | i ) & 0xFFFF  ));
        if(nk_selectable_label(ctx, hexString, NK_TEXT_LEFT, &selected)) {
            printf("page is 0x0%X\n", i);
            page = i;
            printf("page is 0x%X00 selected %d\n", i, (int)selected);
        }
    }
}

static char peekString[64];
static int peekLen = 0;


typedef struct InstructionLabel{
    u16 pos;
    char* instruction;
} InstructionLabel;

static inline void
instruction_label(InstructionLabel label, u16 wantedAddr) {

    char reqString[64];
    sprintf ( reqString,"0x%04X", label.pos);

    if(label.pos == breakpoint) {
        int temp = wantedAddr == label.pos;
        if(nk_selectable_label(ctx, reqString, NK_TEXT_LEFT, &temp)) {
            sprintf ( peekString,"%X", label.pos);
            peekLen = strlen(peekString);
        }

        nk_label_colored(ctx, label.instruction, NK_TEXT_LEFT, nk_rgb(0, 0, 200));

    } else if(cpu.pc == label.pos) {

        int temp = wantedAddr == label.pos;
        if(nk_selectable_label(ctx, reqString, NK_TEXT_LEFT, &temp)) {
            sprintf ( peekString,"%X", label.pos);
            peekLen = strlen(peekString);
        }

        //nk_label_colored(ctx, reqString, NK_TEXT_LEFT, nk_rgb(200, 200, 0));
        nk_label_colored(ctx, label.instruction, NK_TEXT_LEFT, nk_rgb(200, 200, 0));

    } else if(wantedAddr == label.pos) {
        int temp = 1;
        nk_selectable_label(ctx, reqString, NK_TEXT_LEFT, &temp);
        //nk_label_colored(ctx, reqString, NK_TEXT_LEFT, nk_rgb(200, 0, 0));
        nk_label_colored(ctx, label.instruction, NK_TEXT_LEFT, nk_rgb(200, 0, 0));
    } else {

        int temp = 0;
        if(nk_selectable_label(ctx, reqString, NK_TEXT_LEFT, &temp)) {
            sprintf ( peekString,"%X", label.pos);
            printf("pressed\n");
            peekLen = strlen(peekString);
        }

        //nk_label(ctx, reqString, NK_TEXT_LEFT);
        nk_label(ctx, label.instruction, NK_TEXT_LEFT);
    }
}

static void
intruction_debugger() {

    float ratio[] = {120, 150};
    nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
    nk_label(ctx, "Hex:", NK_TEXT_LEFT);
    nk_edit_string(ctx, NK_EDIT_SIMPLE, peekString, &peekLen, 5, nk_filter_hex);
    peekString[peekLen] = 0;
    u16 wantedAddr = cpu.pc;

    if(peekLen != 0) {
        wantedAddr = (u16)strtol(peekString, NULL, 16);
    }
    {
        nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
        static char breakString[64];
        static int breakLen = 0;
        nk_label(ctx, "Breakpoint:", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_SIMPLE, breakString, &breakLen, 5, nk_filter_hex);
        breakString[breakLen] = 0;

        if(breakLen != 0) {
            breakpoint = (u16)strtol(breakString, NULL, 16);
        }
    }
    {
        nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
        static char breakString[64] = "0";
        static int breakLen = 1;
        nk_label(ctx, "CountBreakPoint:", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_SIMPLE, breakString, &breakLen, 10, nk_filter_default);
        breakString[breakLen] = 0;

        if(breakLen != 0) {
            instructionCountBreakPoint = (u16)strtol(breakString, NULL, 10);
        } else {
            instructionCountBreakPoint = 0;
        }
    }

#if 1
    char* notKnownOperand = "Outside of PRG mem";
    char* errOp = "ERR OP";

    InstructionLabel instructionCache[14 + 1 + 14] = {0};

    // up
    u16 tempPC = wantedAddr - 1;
    for(i32 i = 13; i >= 0; tempPC--) {

        u16 realAddr = 0; //mapper.cpu_translate_peak(tempPC); // TODO fix
        if(realAddr != 0xFFFF) {
            if(disassemblyTable[realAddr]) {
                instructionCache[i] =
                    (struct InstructionLabel) { .pos = tempPC, .instruction = disassemblyTable[realAddr]};
                i -= 1;
            }
        } else {
            instructionCache[i] =
                (struct InstructionLabel) { .pos = tempPC, .instruction = notKnownOperand};
            i -= 1;
        }
    }

    tempPC = wantedAddr; // Current location
    u16 realAddr = 0; //mapper.cpu_translate_peak(tempPC); TODO fix

    if(realAddr != 0xFFFF) {
        if(disassemblyTable[realAddr]) {
            instructionCache[14] =
                (struct InstructionLabel) { .pos = tempPC, .instruction = disassemblyTable[realAddr]};
        } else {
            instructionCache[14] =
                (struct InstructionLabel) { .pos = tempPC, .instruction = errOp};
        }
    } else {
        // TODO generate lable if now known?
        instructionCache[14] =
            (struct InstructionLabel) { .pos = tempPC, .instruction = notKnownOperand};
    }

    // down
    tempPC = wantedAddr + 1;
    for(u16 i = 15; i < 29; tempPC++) {
        u16 realAddr = 0; //mapper.cpu_translate_peak(tempPC); TODO fix
        if(realAddr != 0xFFFF) {
            if(disassemblyTable[realAddr]) {
                instructionCache[i] =
                    (struct InstructionLabel) { .pos = tempPC, .instruction = disassemblyTable[realAddr]};
                i += 1;
            }
        } else {
            instructionCache[i] =
                (struct InstructionLabel) { .pos = tempPC, .instruction = notKnownOperand};
            i += 1;
        }
    }
#if 0
    LOG("");
    for(u32 i = 0; i < 29; i++) {
        LOG("%s", instructionCache[i].instruction);
    }
    LOG("");
#endif
    for(u16 i = 0; i < 29; i++) {

        nk_layout_row_static(ctx, 20, 100, 2);

        instruction_label(instructionCache[i], wantedAddr);

#if 0
        u32 index = (wantedAddr - 15 + i) & 0xFFFF;

        u32 programMemSize = cartridge.numProgramRoms * PROG_ROM_SINGLE_SIZE;
        u16 realAddr = mapper.cpu_translate_peak(index);
        //bool selected = 0;
        char* text;
        if(realAddr != 0xFFFF) {
            text = disassemblyTable[realAddr];
        } else {
            text = notKnownOperand;
        }
#endif
    }
#endif
}

static void
nametable_debugger() {
    static u32 selectedAttribute = numeric_max_u32;
    static u8  selectedAttributeValue = 0;
    static u32 selectedAttributeIndex = numeric_max_u32;

    nk_layout_row_static(ctx, 15, 200, 1);
    switch(cartridge.mirrorType){
        case VERTICAL:
            {
                nk_label(ctx, "Vertical Mirroring", NK_TEXT_LEFT);
            } break;
        case HORIZONTAL:
            {
                nk_label(ctx, "Horizontal Mirroring", NK_TEXT_LEFT);
            } break;
        default:
            ABORT("unknown mirroring type");

    }

    nk_layout_row_static(ctx, 15, 200, 2);
    static int active = 1;

    active ^= 1;
    if(nk_checkbox_label(ctx, "First", &active)) {
        selectedAttribute = numeric_max_u32;
        selectedAttributeValue = 0;
    }
    active ^= 1;
    if(nk_checkbox_label(ctx, "Second", &active)) {
        selectedAttribute = numeric_max_u32;
        selectedAttributeValue = 0;
    }

    nk_layout_row_static(ctx, 15, 20, 32);

    char tableIdString[32];
    // BG IDs
    for(u32 y = 0; y < 30; y++) {
        u32 attributeY = y / 4;
        for(u32 x = 0; x < 32; x++) {

            u32 attributeX = x / 4;

            sprintf(tableIdString, "%02X",
                    ppu.nameTables[(NAMETABLE_SIZE * (active ^ 1)) + y * 32 + x]);

            if((attributeY * (32 / 4) + attributeX) == selectedAttribute) {
                //[0x00][0x01]
                //[0x10][0x11]
                u32 temp = (((y / 2) & 0x1) << 1) |  (((x / 2) & 0x1));
                if(temp == selectedAttributeIndex) {
                    nk_label_colored(ctx, tableIdString, NK_TEXT_LEFT, nk_rgb(0, 200, 200));
                } else {
                    nk_label_colored(ctx, tableIdString, NK_TEXT_LEFT, nk_rgb(200, 200, 0));
                }
            } else {
                nk_label(ctx, tableIdString, NK_TEXT_LEFT);
            }

        }
    }

    nk_layout_row_static(ctx, 15, 200, 1);
    nk_label(ctx, "Attribute Table", NK_TEXT_LEFT);

    nk_layout_row_static(ctx, 15, 20, 32);
    // Palettes IDs
    for(u32 y = 0; y < 2; y++) {
        for(u32 x = 0; x < 32; x++) {
            sprintf(tableIdString, "%02X",
                    ppu.nameTables[(NAMETABLE_SIZE * (active ^ 1)) + (y + 30) * 32 + x]);
            i32 selected = (y * 32 + x) == selectedAttribute;
            //nk_label(ctx, tableIdString, NK_TEXT_LEFT);

            if(nk_selectable_label(ctx, tableIdString, NK_TEXT_LEFT, &selected)) {
                //LOG("Attrinute table 0x0%X", y * 32 + x);
                selectedAttribute = selected ? (y * 32 + x) : numeric_max_u32;
                selectedAttributeValue = selected ?
                    ppu.nameTables[(NAMETABLE_SIZE * (active ^ 1)) + (y + 30) * 32 + x] : 0;
            }
        }
    }

    char binaryText[32];
    nk_layout_row_static(ctx, 15, 200, 1);
    nk_label(ctx, "Attribute View", NK_TEXT_LEFT);
    u8 tempAttribVal = selectedAttributeValue;

    nk_layout_row_static(ctx, 15, 20, 2);
    for(u32 i = 0; i < 4; i++) {
        u8 paletteID = tempAttribVal & 0x3;
        sprintf(binaryText, "%c%c", paletteID & 0x2 ? '1' : '0', paletteID & 0x1 ? '1' : '0');
        i32 selected = i == selectedAttributeIndex;
        if(nk_selectable_label(ctx, binaryText, NK_TEXT_LEFT, &selected)) {
            selectedAttributeIndex = selected ? i : numeric_max_u32;
        }
        tempAttribVal >>= 2;
    }
}

static void
pattern_view() {

    //nk_layout_row_begin(ctx, NK_STATIC, ppu.pattern[0].h, 2);

    static int palette;

    nk_layout_row_begin(ctx, NK_STATIC, 22, 1);
    nk_layout_row_push(ctx, 130);

    nk_property_int(ctx, "#Palette:", 0, &palette, 7, 1, 1);

    ppu_render_patterntable(0, palette % 8);
    imageview_update(&ppu.pattern[0]);

    ppu_render_patterntable(1, palette % 8);
    imageview_update(&ppu.pattern[1]);

    int w = ppu.pattern[0].h * 2, h = ppu.pattern[0].h * 2;
    nk_layout_space_begin(ctx, NK_STATIC, h, 2);

    nk_layout_space_push(ctx, nk_rect(0, 0, w, h));

    nk_image(ctx, patternImage[0]);

    nk_layout_space_push(ctx, nk_rect(w + 10, 0, w, h));

    nk_image(ctx, patternImage[1]);

    nk_layout_space_end(ctx);

}

static void
oam_view() {

    ppu_render_oam();
    imageview_update(&ppu.OAMvisualisation);

    int w = ppu.OAMvisualisation.h * 2, h = ppu.OAMvisualisation.h * 2;
    nk_layout_space_begin(ctx, NK_STATIC, h, 2);

    nk_layout_space_push(ctx, nk_rect(0, 0, w, h));

    nk_image(ctx, oamImage);

    nk_layout_space_end(ctx);

}

int step = 0, frameSkip = 0;
static void
debugger_update() {

#if 0
    u32 windowFlags = 0;
    if (nk_begin(ctx, "General", nk_rect(0, 0, (SCREEN_WIDTH * 0.75), SCREEN_HEIGHT), windowFlags)) {

        if (nk_tree_push(ctx, NK_TREE_TAB, "Memory debugger", NK_MINIMIZED)) {
            memory_debugger();
            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, "Pattern view", NK_MINIMIZED)) {
            pattern_view();
            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, "OAM view", NK_MINIMIZED)) {
            oam_view();
            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, "Nametable debugger", NK_MINIMIZED)) {
            nametable_debugger();
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);

    if (nk_begin(ctx, "Side Bar", nk_rect((SCREEN_WIDTH * 0.75), 0, (SCREEN_WIDTH * 0.25), SCREEN_HEIGHT), windowFlags)) {

        nk_layout_row_static(ctx, 30, 80, 1);

        if(!debug) {
            if(nk_button_label(ctx, "Step")) {
                step = 1;
            }
        }

        if(nk_button_label(ctx, "Reset")) {
            cpu_reset();
        }

        if(nk_checkbox_label(ctx, "Debug", &debug)) {
            frameSkip = 0;
        }

        if(nk_checkbox_label(ctx, "FrameSkip", &frameSkip)) {
            debug = 0;
        }
        intruction_debugger();
    }

    nk_end(ctx);
#endif
    u32 posX = (u32)(SCREEN_WIDTH * 0.25);
    u32 posX2 = (u32)(SCREEN_WIDTH * 0.50);
    u32 posY = (u32)(SCREEN_HEIGHT * 0.25);
    u32 posY2 = (u32)(SCREEN_HEIGHT * 0.50);
    if (nk_begin(ctx, "Game view", nk_rect(posX, posY, posX2, posY2),
                NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_NO_SCROLLBAR|
                NK_WINDOW_BORDER|NK_WINDOW_TITLE))
    {
        struct nk_rect bounds;
        bounds = nk_window_get_bounds(ctx);

        if(bounds.h > 60)
            bounds.h -= 60;


        if(bounds.w > 15)
            bounds.w -= 15;

        float target = (float)TEX_WIDTH/ (float)TEX_HEIGHT;
        float ratio = (float)bounds.w/ (float)bounds.h;

        if(ratio > target) { // larger width

            nk_layout_space_begin(ctx, NK_STATIC, bounds.h, 1);
            int gap = (bounds.w - bounds.h * target) / 2;
            nk_layout_space_push(ctx, nk_rect(gap, 0, bounds.h *  target, bounds.h));
            nk_image(ctx, renderImage);

            nk_layout_space_end(ctx);
        } else { // larger height

            nk_layout_space_begin(ctx, NK_STATIC, bounds.h, 1);

            target = (float)TEX_HEIGHT/ (float)TEX_WIDTH;
            int gap = (bounds.h - bounds.w * target) / 2;
            nk_layout_space_push(ctx, nk_rect(0, gap, bounds.w, bounds.w *  target));
            nk_image(ctx, renderImage);

            nk_layout_space_end(ctx);
        }

    }
    if(spacePressed) {
        spacePressed = 0;
        if(nk_window_has_focus(ctx)) {
            debug ^= 1;
            LOG("DEBUG SET");
        } else {
            LOG("GAME FOCUCED");
            nk_window_set_focus(ctx, "Game view");
        }
    }
    nk_end(ctx);

    //overview(ctx);
}

static void
debugger_draw() {
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
}
#endif
