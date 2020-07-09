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
#include "cpu.h"
#include "ppu.h"
#include "input.h"
#include "debugger.h"


int
main(int argc, char** argv) {
    printf("hello\n");

    SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );


    if(!SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1")) {
        printf("not set!\n");
    }

    if(!SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1")) {
        printf("not set!\n");
    }

    SDL_Window *window = SDL_CreateWindow("nes-emu",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    assert(window);
    SDL_GLContext Context = SDL_GL_CreateContext(window); (void) Context;

    if(argc == 2) {
        cartridge_load(argv[1]);
    } else {
        cartridge_load("roms/nestest.nes");
    }
    cpu_reset();

    LOG_COLOR(CONSOLE_COLOR_BLUE ,"cpu debugger init");

    ppu_init();
    debugger_init(window);

    LOG_COLOR(CONSOLE_COLOR_BLUE ,"cpu debugger inited");

    int running = 1;
    LOG("All initialized");
    LOG_COLOR(CONSOLE_COLOR_GREEN ,"All initialized");

    init_gamepad();

    LOG("All initialized");

    u32 updateCounter = 0;

    u32 currentTime;
    u32 lastTime = currentTime = SDL_GetTicks();

    double targetDelta = 1.0 / 60.0;

    while (running) {

        running = keystate_update();

        if(debug == 0) { //debug update
            if (frameSkip && step) {
                LOG("FRAMESKIPPING");
                do {
                    ppu_clock();
                    if(updateCounter % 3 == 0) {
                        if(!ppu.oam.DMAactive) {
                            cpu_clock();
                        }
                        else {
                            ppu_dma_oam(updateCounter);
                        }
                    }
                    if(ppu.NMIGenerated == 1) {
                        ppu.NMIGenerated = 0;
                        cpu_no_mask_iterrupt();
                    }

                    updateCounter += 1;
                } while(ppu.frameComplete == 0);
                ppu.frameComplete = 0;
                step = 0;
            } else if (step) {
                if(step) {
                    LOG("DEBUGGING");
                    u8 updated = 0;
                    do {
                        ppu_clock();
                        if(updateCounter % 3 == 0) {

                            if(!ppu.oam.DMAactive) {
                                updated = cpu_clock();
                            } else {
                                ppu_dma_oam(updateCounter);
                            }

                        }
                        if(ppu.NMIGenerated == 1) {
                            ppu.NMIGenerated = 0;
                            cpu_no_mask_iterrupt();
                        }

                        updateCounter += 1;
                    } while(updated != 1);
                    step = 0;
                }
            }
        } else { //normal update

            currentTime = SDL_GetTicks();
            double delta = (double)(currentTime - lastTime) / 1000.0;

            if(delta > targetDelta) {
                lastTime = currentTime;

                do {
                    ppu_clock();
                    if(updateCounter % 3 == 0) {
                        if(!ppu.oam.DMAactive) {
                            cpu_clock();
                        } else {
                            ppu_dma_oam(updateCounter);
                        }
                    }
                    if(ppu.NMIGenerated == 1) {
                        ppu.NMIGenerated = 0;
                        cpu_no_mask_iterrupt();
                    }
                    updateCounter += 1;
                } while(ppu.frameComplete == 0 && debug == 1);
                ppu.frameComplete = 0;
            }
        }
        debugger_update();

        i32 winWidth, winHeight;
        SDL_GetWindowSize(window, &winWidth, &winHeight);
        glViewport(0, 0, winWidth, winHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor( 0, 0, 0, 0);
        debugger_draw();

        ppu_render();

        SDL_GL_SwapWindow(window);
    }

    nk_sdl_shutdown();
    //TODO clean everything up
    LOG("everything shutdown correctly...");
}
