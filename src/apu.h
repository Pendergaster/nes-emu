/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef APU_H
#define APU_H

#include <SDL2/SDL.h>
#include "defs.h"

#define SAMPLES_PER_SECOND  44100
#define SAMPLE_BUFFER_SIZE  1024

typedef struct Pulse {
    u16 time;
} Pulse;

struct APU {
    Pulse pulses[2];
} apu;

static void
pulse_write(Pulse* pulse, u16 addr, u8 val) {
    switch(addr) {

        case 0x4002:
            {
                // Time low bits
                pulse->time &= 0xFF00;
                pulse->time |= val;
            } break;
        case 0x4003:
            {
                // Time high bits
                pulse->time &= 0x00FF;
                pulse->time |= val;
            } break;
        default:
            ABORT("Error address in pulse 0x%04X", addr);
    }
}

static void
apu_init() {

    //SDL_setenv("SDL_AUDIODRIVER", "disk", 1);
    SDL_setenv("SDL_AUDIODRIVER", "directsound", 1);

    SDL_Init(SDL_INIT_AUDIO);

    // Open our audio device; Sample Rate will dictate the pace of our synthesizer
    //SDLInitAudio(44100, 1024);

    SDL_AudioSpec AudioSettings = { 0 };

    AudioSettings.freq = SAMPLES_PER_SECOND;
    //  One of the modes that doesn't produce a high frequent pitched tone when having silence
    AudioSettings.format = AUDIO_F32SYS;
    AudioSettings.channels = 2;
    AudioSettings.samples = SAMPLE_BUFFER_SIZE;

    SDL_OpenAudio(&AudioSettings, 0);

    //if (!SoundIsPlaying)

    {
        // start playing the audio
        SDL_PauseAudio(0);
        //SoundIsPlaying = true;
    }
}

static void
apu_write(u16 addr, u8 data) {

    if(address_is_between(addr, 0x4000, 0x4003)) {
        pulse_write(&apu.pulses[0],addr, data);
    } else {
        LOG("apu todo address implementation");
    }
}

#endif /* APU_H */
