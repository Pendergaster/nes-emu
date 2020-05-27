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

#define INCLUDE_ALL
/*#define INCLUDE_STYLE */
/*#define INCLUDE_CALCULATOR */
/*#define INCLUDE_OVERVIEW */
/*#define INCLUDE_NODE_EDITOR */

#ifdef INCLUDE_ALL
//#define INCLUDE_CALCULATOR
//#define INCLUDE_NODE_EDITOR
#define INCLUDE_OVERVIEW
//#define INCLUDE_STYLE
#endif

#ifdef INCLUDE_STYLE
//#include "style.c"
#endif
#ifdef INCLUDE_CALCULATOR
//#include "calculator.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
//#include "node_editor.c"
#endif
#ifdef INCLUDE_OVERVIEW
#include "overview.c"
#endif

/* GUI */
struct nk_context *ctx;
struct nk_colorf bg;

#define CREATE_CPU_TABLE_STING(NAME, ADDR, XXX) \
#NAME " " #ADDR,

//##ADDR##,
char* cpuInstructionStrings[] = {
    INSTRUCTION_TABLE(CREATE_CPU_TABLE_STING)
};

char** disassemblyTable;

static void
cpu_debugger_init(SDL_Window *win) {

    // Disasseble all memory

    disassemblyTable = (char**)calloc(0x10000, sizeof(char*));

    for(int i = 0; i <= 0xFFFF; i++) {
        char* disassembly = malloc(32);
        int line = i;
        u8 opcode = bus_read8(i);

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
            low = bus_read8(i);

            i++;
            high = bus_read8(i);
        }
        else if( instruction.addressMode == IMP || // fetch 0 addresses
                instruction.addressMode == ACCUM //||
                //instruction.addressMode == IMM
               )
        {
            disassembly[instLen + 1] = 0;
            disassemblyTable[line] = disassembly;
            continue;
        }
        else // fetch low adress
        {
            i++;
            low = bus_read8(i);
        }

        u16 holeAddr = (high << 8) | low;
        char temp[32];

        sprintf(temp, "0x%04X", holeAddr);
#if 0
        char* hexString = "0x0000";
        char* hex = "0123456789ABCDEF";

        u16 index = 2 + ((holeAddr >> 12) & 0xF);
        hexString[2 + index] = hex[index];

        index = 2 + ((holeAddr >> 8) & 0xF);
        hexString[2 + index] = hex[index];

        index = 2 + ((holeAddr >> 4) & 0xF);
        hexString[2 + index] = hex[index];

        index = 2 + ((holeAddr) & 0xF);
        hexString[2 + index] = hex[index];
#endif
        int hexStringSize = strlen(temp) + 1;
        memcpy(&disassembly[instLen + 1], temp, hexStringSize);

        disassemblyTable[line] = disassembly;
    }


#if 0
    for(int i = 0; i < SIZEOF_ARRAY(cpuInstructionStrings); i++) {
        printf("In %s\n", cpuInstructionStrings[i]);
    }
#endif

    ctx = nk_sdl_init(win);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */

    {struct nk_font_atlas *atlas;
        nk_sdl_font_stash_begin(&atlas);

        /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
        /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
        /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
        /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
        /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
        /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
        nk_sdl_font_stash_end();
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        /*nk_style_set_font(ctx, &roboto->handle);*/}

    /* style.c */
#ifdef INCLUDE_STYLE
    /*set_style(ctx, THEME_WHITE);*/
    /*set_style(ctx, THEME_RED);*/
    /*set_style(ctx, THEME_BLUE);*/
    /*set_style(ctx, THEME_DARK);*/
#endif
}

static void
cpu_memory_debugger() {

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

    if (nk_begin(ctx, "Cpu Memory Debugger", nk_rect(0, 0, 700, 800),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        static u32 page;
#if 0
        unsigned char pageString[32];

        //nk_layout_row_dynamic(ctx, 25, 3);

        struct nk_rect bounds;
#if 0
        nk_label(ctx, "Right Click here:", NK_TEXT_LEFT);

        nk_button_color(ctx, nk_rgb(200, 200, 0));
#endif

        nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
        nk_layout_row_push(ctx, 25);
        if(nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT) && page > 0) {
            page -= 1;
        }
        nk_layout_row_push(ctx, 65);
#if 0
        bounds = nk_widget_bounds(ctx);
#endif

        sprintf ( pageString,"Page 0x%X0", page);
        nk_label(ctx, pageString, NK_TEXT_LEFT);
        nk_layout_row_push(ctx, 25);


        if(nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT) && page < 0xF) {
            page += 1;
        }
#endif
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

            sprintf ( hexString,"0x%X", (bus_read8( (page << 8) | i ) & 0xFFFF ));
            if(nk_selectable_label(ctx, hexString, NK_TEXT_LEFT, &selected)) {
                printf("page is 0x0%X\n", i);
                page = i;
                printf("page is 0x%X00 selected %d\n", i, (int)selected);
            }
        }



#if 0
        enum {EASY, HARD};
        static int op = EASY;
        static int property = 20;

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Label aligned left", NK_TEXT_CENTERED);

        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "button"))
            printf("button pressed!\n");
        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
        if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "background:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
            nk_layout_row_dynamic(ctx, 120, 1);
            bg = nk_color_picker(ctx, bg, NK_RGBA);
            nk_layout_row_dynamic(ctx, 25, 1);
            bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
            bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
            bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
            bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
            nk_combo_end(ctx);
        }
#endif
    }
    nk_end(ctx);
}

static void
cpu_intruction_debugger() {

    if (nk_begin(ctx, "Cpu instructions Debugger", nk_rect(700, 0, 300, 900),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {

        u16 pc = cpu.pc;
        float ratio[] = {120, 150};
        nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
        static char peekString[64];
        static int peekLen = 0;
        nk_label(ctx, "Hex:", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_SIMPLE, peekString, &peekLen, 5, nk_filter_hex);
        peekString[peekLen] = 0;

        if(peekLen != 0) {
            pc = (u16)strtol(peekString, NULL, 16);
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



        char reqString[64];

        for(u16 i = 0; i < 30; i++) {

            nk_layout_row_static(ctx, 20, 100, 3);

            int index = (pc - 15 + i) & 0xFFFF;
            sprintf ( reqString,"0x%04X", index);
            //bool selected = 0;
            if(index == breakpoint) {

                int temp =  pc == index;
                if(nk_selectable_label(ctx, reqString, NK_TEXT_LEFT, &temp)) {
                    sprintf ( peekString,"%X", index);
                    peekLen = strlen(peekString);
                }

                if(disassemblyTable[index]) {
                    nk_label_colored(ctx, disassemblyTable[index], NK_TEXT_LEFT, nk_rgb(0, 0, 200));
                }

            } else if(cpu.pc == index) {

                int temp =  pc == index;
                if(nk_selectable_label(ctx, reqString, NK_TEXT_LEFT, &temp)) {
                    sprintf ( peekString,"%X", index);
                    peekLen = strlen(peekString);
                }


                //nk_label_colored(ctx, reqString, NK_TEXT_LEFT, nk_rgb(200, 200, 0));
                if(disassemblyTable[index]) {
                    nk_label_colored(ctx, disassemblyTable[index], NK_TEXT_LEFT, nk_rgb(200, 200, 0));
                }
            } else if(pc == index) {
                int temp = 1;
                nk_selectable_label(ctx, reqString, NK_TEXT_LEFT, &temp);
                //nk_label_colored(ctx, reqString, NK_TEXT_LEFT, nk_rgb(200, 0, 0));
                if(disassemblyTable[index]) {
                    nk_label_colored(ctx, disassemblyTable[index], NK_TEXT_LEFT, nk_rgb(200, 0, 0));
                }
            } else {

                int temp = 0;
                if(nk_selectable_label(ctx, reqString, NK_TEXT_LEFT, &temp)) {
                    sprintf ( peekString,"%X", index);
                    printf("pressed\n");
                    peekLen = strlen(peekString);
                }

                //nk_label(ctx, reqString, NK_TEXT_LEFT);
                if(disassemblyTable[index]) {
                    nk_label(ctx, disassemblyTable[index], NK_TEXT_LEFT);
                }
            }
        }
    }

    nk_end(ctx);
}

int step = 0, hide = 0;
static void
cpu_debugger_update() {

#if 1
    if(hide == 0) {
        cpu_memory_debugger();

        cpu_intruction_debugger();
    }

    if (nk_begin(ctx, "Cpu menu", nk_rect(SCREEN_WIDTH - 300, 0, 300, 300),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        nk_layout_row_static(ctx, 30, 80, 1);

        if(hide == 0) {
            if(nk_button_label(ctx, "Step")) {
                step = 1;
            }

            if(nk_button_label(ctx, "Reset")) {
                cpu_reset();
            }

            if(nk_button_label(ctx, "IRQ")) {
                cpu_iterrupt_request();
            }

            if(nk_button_label(ctx, "INM")) {
                cpu_no_mask_iterrupt();
            }
        }

        nk_checkbox_label(ctx, "Debug", &debug);
        nk_checkbox_label(ctx, "Hide", &hide);
    }
    nk_end(ctx);
#endif


    /* -------------- EXAMPLES ---------------- */
#ifdef INCLUDE_CALCULATOR
    //calculator(ctx);
#endif
#ifdef INCLUDE_OVERVIEW
    //overview(ctx);
#endif
#ifdef INCLUDE_NODE_EDITOR
    //node_editor(ctx);
#endif
    /* ----------------------------------------- */
}

static void
cpu_debugger_draw() {
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
}
#endif
