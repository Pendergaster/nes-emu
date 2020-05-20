/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#include <stdio.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

#include "defs.h"

const u32 SCREEN_WIDTH = 1800, SCREEN_HEIGHT = 1000;

#include "printutils.h"
#include "fileload.h"
#include "cartridge.h"
#include "bus.h"
#include "cpu2ao3.h"
#include "ppu.h"
#include "cpudebugger.h"
#include "ppudebugger.h"


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
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    assert(window);
    SDL_GLContext Context = SDL_GL_CreateContext(window);
    (void) Context;
#if 0
    const char *my_str_literal = "A2 0A 8E 00 00 A2 03 8E 01 00 AC 00 00 A9 00 18 6D 01 00 88 D0 FA 8D 02 00 EA EA EA";
    //"JAN,FEB,MAR";
    char *token, *str, *tofree;
    u8* rom = calloc(200, 1);

    tofree = str = strdup(my_str_literal);  // We own str's memory now.
    int romi = 0;
    while ((token = strsep(&str, " "))) rom[romi++] = (u8)strtol(token, NULL, 16);
    free(tofree);

    LOG_COLOR(CONSOLE_COLOR_BLUE ,"loading rom");

    cpu_load_rom(rom, romi, 0x08000);
    LOG_COLOR(CONSOLE_COLOR_BLUE ,"rom loaded");
#endif

    cartridge_load("roms/nestest.nes");
    cpu_reset();

    LOG_COLOR(CONSOLE_COLOR_BLUE ,"cpu debugger init");

    ppu_init();
    cpu_debugger_init(window);
    ppu_debugger_init();

    LOG_COLOR(CONSOLE_COLOR_BLUE ,"cpu debugger inited");

    int running = 1;
    SDL_Event event;
    LOG("All initialized");
    LOG_COLOR(CONSOLE_COLOR_GREEN ,"All initialized");

    //exit(1);

    LOG("All initialized");

    u32 updateCounter = 0;
    while (running) {

        nk_input_begin(ctx);
        while (SDL_PollEvent(&event)) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    if (event.type == SDL_KEYDOWN) running = 0;
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

        if(debug == 0) { //debug update
            if(step) {
                if (frameSkip) {
                    do {
                        ppu_clock();
                        if(updateCounter % 3 == 0) {
                            cpu_clock();
                        }
                        updateCounter += 1;
                    } while(ppu.frameComplete == 0);
                } else {
                    u8 updated = 0;
                    do {
                        ppu_clock();
                        if(updateCounter % 3 == 0) {
                            updated = cpu_clock();
                        }

                        updateCounter += 1;
                    } while(updated != 1);
                }

            }
            step = 0;
        } else { //normal update
            do {
                ppu_clock();
                if(updateCounter % 3 == 0) {
                    cpu_clock();
                }

                updateCounter += 1;
            } while(ppu.frameComplete == 0);
            ppu.frameComplete = 0;
        }
        cpu_debugger_update();

        i32 winWidth, winHeight;
        SDL_GetWindowSize(window, &winWidth, &winHeight);
        glViewport(0, 0, winWidth, winHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor( 200, 0, 0, 0);
        cpu_debugger_draw();
        ppu_debugger_draw();

        ppu_render();

        SDL_GL_SwapWindow(window);
    }

    nk_sdl_shutdown();
    //TODO clean everything up
    LOG("byeee");
}
