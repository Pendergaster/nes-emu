/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */
#include <stdio.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include "defs.h"
//#include "printutils.h"

#include "cpu2ao3.h"

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

    SDL_Window *window = SDL_CreateWindow("Chip8",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    assert(window);
    SDL_GLContext Context = SDL_GL_CreateContext(window);

    int running = 1;
    SDL_Event event;
    //LOG("All initialized");
    //LOG_COLOR(CONSOLE_COLOR_GREEN ,"All initialized");
    //LOG_COLOR(CONSOLE_COLOR_BLUE ,"All initialized");
    //LOG("All initialized");

    while (running) {

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
        }
    }
}
