/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#include <stdio.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include "defs.h"
#include "printutils.h"
#include "fileload.h"

#include "bus.h"
#include "cpu2ao3.h"
#include "cartridge.h"
#include "ppu.h"
#include "cpudebugger.h"

const u32 width = 1000, height = 800;

int
main() {
    printf("hello\n");

    SDL_Init( SDL_INIT_VIDEO );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    if(!SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1")) {
        printf("not set!\n");
    }

    SDL_Window *window = SDL_CreateWindow("nes-emu",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    assert(window);
    SDL_GLContext Context = SDL_GL_CreateContext(window);
    (void) Context;
#if 0
    size_t size;
    u8* rom = load_binary_file("asm_test", &size);
    cpu_load_rom(rom, size, 0x8000);
    char* romdata =
    char** list = string_split(romdata, ' ');
#else

    const char *my_str_literal = "A2 0A 8E 00 00 A2 03 8E 01 00 AC 00 00 A9 00 18 6D 01 00 88 D0 FA 8D 02 00 EA EA EA";
        //"JAN,FEB,MAR";
    char *token, *str, *tofree;
    u8* rom = calloc(200, 1);

    tofree = str = strdup(my_str_literal);  // We own str's memory now.
    int romi = 0;
    while ((token = strsep(&str, " "))) rom[romi++] = (u8)strtol(token, NULL, 16);
    free(tofree);


    cpu_load_rom(rom, romi, 0x8000);

#if 0

    int i;
    for (i = 0; *(list + i); i++) {

    }
#endif
#endif
    cpu_reset();

    LOG_COLOR(CONSOLE_COLOR_BLUE ,"cpu debugger init");

    cpu_debugger_init(window);

    LOG_COLOR(CONSOLE_COLOR_BLUE ,"cpu debugger inited");

    int running = 1;
    SDL_Event event;
    LOG("All initialized");
    LOG_COLOR(CONSOLE_COLOR_GREEN ,"All initialized");
    LOG("All initialized");
    while (running) {

        nk_input_begin(ctx);
        while (SDL_PollEvent(&event)) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:

                    running = 0;

                    break;
                default:
                    break;
            }
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            nk_sdl_handle_event(&event);
        }
        nk_input_end(ctx);
        if(debug == 0 && step) {
            while(cpu_clock() == 0);
            step = 0;
        }
        cpu_debugger_update();

        i32 winWidth, winHeight;
        SDL_GetWindowSize(window, &winWidth, &winHeight);
        glViewport(0, 0, winWidth, winHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bg.r, bg.g, bg.b, bg.a);
        cpu_debugger_draw();

        SDL_GL_SwapWindow(window);
    }
    LOG("byeee");
}
