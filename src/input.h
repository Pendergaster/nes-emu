/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef INPUT_H
#define INPUT_H

static void gamepad_init() {

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
    KEY_RIGHT   = 0x01,
    KEY_LEFT    = 0x02,
    KEY_DOWN    = 0x04,
    KEY_UP      = 0x08,
    KEY_START   = 0x10,
    KEY_SELECT  = 0x20,
    KEY_B       = 0x40,
    KEY_A       = 0x80,
};


static inline void
set_button(u32 controller, u32 key, u32 state) {

    if(state == SDL_JOYBUTTONDOWN || state == SDL_JOYBUTTONUP ||
            state == SDL_KEYDOWN || state == SDL_KEYUP) {

        internalButtonState[controller] =
            (internalButtonState[controller] & (~key));

        if(state == SDL_JOYBUTTONDOWN || state == SDL_KEYDOWN) {
            internalButtonState[controller] |= key;
        }
    }
}

struct nk_context *ctx;
void nk_input_begin(struct nk_context *ctx);
int nk_sdl_handle_event(SDL_Event *evt);
void nk_input_end(struct nk_context *ctx);

u32 spacePressed;

static u32 keystate_update() {

    // nk_find_window(struct nk_context *ctx, nk_hash hash, const char *name)

    nk_input_begin(ctx);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                if (event.type == SDL_KEYDOWN) return 0;
                break;
            case SDLK_a:
                set_button(0, KEY_A, event.type);
                break;
            case SDLK_s:
                set_button(0, KEY_B, event.type);
                break;
            case SDLK_z:
                set_button(0, KEY_SELECT, event.type);
                break;
            case SDLK_x:
                set_button(0, KEY_START, event.type);
                break;
            case SDLK_DOWN:
                set_button(0, KEY_DOWN, event.type);
                break;
            case SDLK_UP:
                set_button(0, KEY_UP, event.type);
                break;
            case SDLK_RIGHT:
                set_button(0, KEY_RIGHT, event.type);
                break;
            case SDLK_LEFT:
                set_button(0, KEY_LEFT, event.type);
                break;
            case SDLK_SPACE:
                if(event.type == SDL_KEYDOWN) {
                    spacePressed = 1;
                } break;
            default:
                break;
        }

        if(event.jdevice.which < 2) {
            if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {

                if(event.jbutton.button == 1) { // A button
                    set_button(event.jdevice.which, KEY_A, event.type);
                }
                if(event.jbutton.button == 2) { // B button
                    set_button(event.jdevice.which, KEY_B, event.type);
                }
                if(event.jbutton.button == 8) { // Select button
                    set_button(event.jdevice.which, KEY_SELECT, event.type);
                }
                if(event.jbutton.button == 9) { // Start button
                    set_button(event.jdevice.which, KEY_START, event.type);
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
                } else if (event.jaxis.axis == 1) {
                    internalButtonState[event.jdevice.which] =
                        internalButtonState[event.jdevice.which] & (~(KEY_UP | KEY_DOWN));

                    if(event.jaxis.value < 0) { // Up
                        set_button(event.jdevice.which, KEY_UP, SDL_JOYBUTTONDOWN);
                    } else if (event.jaxis.value > 0) { // Down
                        set_button(event.jdevice.which, KEY_DOWN, SDL_JOYBUTTONDOWN);
                    }
                }
            }
        }

        if(event.jdevice.type == SDL_JOYDEVICEADDED ) {
            LOG("JOY added! %d", event.jdevice.which);
        }



        if (event.type == SDL_QUIT) {
            return 0;
        }
        nk_sdl_handle_event(&event);
    }
    nk_input_end(ctx);

    return 1;
}

#endif /* INPUT_H */
