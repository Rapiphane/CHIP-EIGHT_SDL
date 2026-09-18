#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "chip8.h"

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;
} sdl_w;

// Config Emulator
typedef struct
{
    uint32_t window_height;
    uint32_t window_width;
    uint32_t fg_color;
    uint32_t bg_color;
    uint32_t scale_factor;
    uint32_t ips; // Inst per second
    uint32_t wave_freq;
    uint32_t audio_sample_rate;
    int32_t volume;
    float color_lerp_rate;
} config_w;

bool init_sdl(sdl_w *sdl, config_w *config);
bool set_initial_config(config_w *config, int argc, char *argv[]);
void final_cleanup(sdl_w *sdl);
void clear_screen(sdl_w *sdl, config_w *config);
void update_screen(sdl_w *sdl, chip8_w *chip8, config_w *config);
void update_timers(sdl_w sdl, chip8_w *chip8);
#endif