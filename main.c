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

static void init_gamepad() {

    int num_joysticks = SDL_NumJoysticks();
    for(int i = 0; i < num_joysticks; ++i) {
        SDL_Joystick* js = SDL_JoystickOpen(i);
        if (js) {
            SDL_JoystickGUID guid = SDL_JoystickGetGUID(js);
            char guid_str[1024];
            SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
            const char* name = SDL_JoystickName(js);

            int num_axes = SDL_JoystickNumAxes(js);
            int num_buttons = SDL_JoystickNumButtons(js);
            int num_hats = SDL_JoystickNumHats(js);
            int num_balls = SDL_JoystickNumBalls(js);

            printf("guid str: %s name:%s axes:%d buttons:%d hats:%d balls:%d\n",
                    guid_str, name,
                    num_axes, num_buttons, num_hats, num_balls);
        }
    }
}

enum NES_KEYCODES {
    KEY_A       = 0x80,
    KEY_SELECT  = 0x20,
    KEY_B       = 0x40,
    KEY_START   = 0x10,
    KEY_RIGHT   = 0x01,
    KEY_LEFT    = 0x02,
    KEY_DOWN    = 0x04,
    KEY_UP      = 0x08,
};
/*
0 - A
1 - B
2 - Select
3 - Start
4 - Up
5 - Down
6 - Left
7 - Right
*/
static inline void
set_button(u32 controller, u32 key, u32 state) {

    internalButtonState[controller] =
        (internalButtonState[controller] & (~key));

    if(state == SDL_JOYBUTTONDOWN) {
        internalButtonState[controller] |= key;
    }

    LOG("Button code 0x%04X", internalButtonState[controller]);

}


static u32 keystate_update() {

    nk_input_begin(ctx);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                if (event.type == SDL_KEYDOWN) return 0;
                break;
            default:
                break;
        }

        if(event.jdevice.which < 2) {
            if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {
                //LOG("JOY down %d event %d", event.jdevice.which, event.jbutton.button);

                if(event.jbutton.button == 1) { // A button
                    set_button(event.jdevice.which, KEY_A, event.type);
                    LOG("A button event");
                }
                if(event.jbutton.button == 2) { // B button
                    set_button(event.jdevice.which, KEY_B, event.type);
                    LOG("B button event");
                }
                if(event.jbutton.button == 8) { // Select button
                    set_button(event.jdevice.which, KEY_SELECT, event.type);
                    LOG("Select button event");
                }
                if(event.jbutton.button == 9) { // Start button
                    set_button(event.jdevice.which, KEY_START, event.type);
                    LOG("Start button event");
                }
            }
            if (event.type == SDL_JOYAXISMOTION) {
                if(event.jaxis.axis == 0) {
                    internalButtonState[event.jdevice.which] =
                        internalButtonState[event.jdevice.which] & (~(KEY_LEFT | KEY_RIGHT));

                    if(event.jaxis.value < 0) { // Left
                        set_button(event.jdevice.which, KEY_LEFT, SDL_JOYBUTTONDOWN);
                    } else if (event.jaxis.value > 0) { // Right
                        set_button(event.jdevice.which, KEY_RIGHT, SDL_JOYBUTTONDOWN);
                    }
                    LOG("Axis event 0");
                } else if (event.jaxis.axis == 1) {
                    internalButtonState[event.jdevice.which] =
                        internalButtonState[event.jdevice.which] & (~(KEY_UP | KEY_DOWN));

                    if(event.jaxis.value < 0) { // Up
                        set_button(event.jdevice.which, KEY_UP, SDL_JOYBUTTONDOWN);
                    } else if (event.jaxis.value > 0) { // Down
                        set_button(event.jdevice.which, KEY_DOWN, SDL_JOYBUTTONDOWN);
                    }
                    LOG("Axis event 1");
                }
            }
        }

        if(event.jdevice.type == SDL_JOYDEVICEADDED ) {
            LOG("JOY added! %d", event.jdevice.which);
        }



        if (event.type == SDL_QUIT) {
            return 0;
        } else if (event.type == SDL_KEYDOWN) {
            LOG("pressed %d", event.key.keysym.sym);
        }
        nk_sdl_handle_event(&event);
    }
    nk_input_end(ctx);

    return 1;
}


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
    cpu_debugger_init(window);
    ppu_debugger_init();

    LOG_COLOR(CONSOLE_COLOR_BLUE ,"cpu debugger inited");

    int running = 1;
    LOG("All initialized");
    LOG_COLOR(CONSOLE_COLOR_GREEN ,"All initialized");

    init_gamepad();

    LOG("All initialized");

    u32 updateCounter = 0;
    while (running) {

        running = keystate_update();

        if(debug == 0) { //debug update
            if (frameSkip && step) {
                LOG("FRAMESKIPPING");
                do {
                    ppu_clock();
                    if(updateCounter % 3 == 0) {
                        cpu_clock();
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
                            updated = cpu_clock();
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
            do {
                ppu_clock();
                if(updateCounter % 3 == 0) {
                    cpu_clock();
                }
                if(ppu.NMIGenerated == 1) {
                    ppu.NMIGenerated = 0;
                    cpu_no_mask_iterrupt();
                }
                updateCounter += 1;
            } while(ppu.frameComplete == 0 && debug == 1);
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
