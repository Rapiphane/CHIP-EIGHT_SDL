#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"
#include "chip8.h"

// Set Initial Config
bool set_initial_config(config_w *config, int argc, char *argv[])
{
    // Set default

    *config = (config_w){
        .window_width = 64,
        .window_height = 32,
        .fg_color = 0xFFFFFFFF,
        .bg_color = 0x000000FF,
        .scale_factor = 20, // 1280*640
        .ips = 700,
        .wave_freq = 400,
        .volume = 3000,
        .audio_sample_rate = 44100,
        .color_lerp_rate = 0.2f,
    };

    for (int i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "--scale-factor", strlen("--scale-factor")) == 0)
        {
            i++;
            config->scale_factor = (uint32_t)strtol(argv[i], NULL, 10);
        }
    }
    return true;
}

uint32_t color_lerp(uint32_t color1, uint32_t color2, float lerp_factor)
{
    uint8_t color1_r = (color1 >> 24) & 0xFF;
    uint8_t color1_g = (color1 >> 16) & 0xFF;
    uint8_t color1_b = (color1 >> 8) & 0xFF;
    uint8_t color1_a = (color1 >> 0) & 0xFF;

    uint8_t color2_r = (color2 >> 24) & 0xFF;
    uint8_t color2_g = (color2 >> 16) & 0xFF;
    uint8_t color2_b = (color2 >> 8) & 0xFF;
    uint8_t color2_a = (color2 >> 0) & 0xFF;

    uint8_t lerp_colour_r = (1 - lerp_factor) * color1_r + lerp_factor * color2_r;
    uint8_t lerp_colour_g = (1 - lerp_factor) * color1_g + lerp_factor * color2_g;
    uint8_t lerp_colour_b = (1 - lerp_factor) * color1_b + lerp_factor * color2_b;
    uint8_t lerp_colour_a = (1 - lerp_factor) * color1_a + lerp_factor * color2_a;

    return (lerp_colour_r << 24 | lerp_colour_g << 16 | lerp_colour_b << 8 | lerp_colour_a);
}
// SDL Audio Callback
void audio_callback(void *userdata, uint8_t *stream, int len)
{
    (void)userdata;
    config_w *config = (config_w *)userdata;

    int16_t *buffer = (int16_t *)stream;

    static uint32_t running_sample_index = 0;

    const int32_t square_wave_period = config->audio_sample_rate / config->wave_freq;

    const int32_t half_square_wave_period = square_wave_period / 2;

    int samples_count = len / sizeof(int16_t);

    for (int i = 0; i < samples_count; i++)
    {
        buffer[i] = ((running_sample_index++ / half_square_wave_period) % 2) ? config->volume : -config->volume;
    }
}
// Initialise SDL Function
bool init_sdl(sdl_w *sdl, config_w *config)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("SDL can't be initialised! %s\n", SDL_GetError());
        return false;
    }
    sdl->window = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config->window_width * config->scale_factor, config->window_height * config->scale_factor, 0);
    if (!sdl->window)
    {
        SDL_Log("Couldn't create window! %s\n", SDL_GetError());
        return false;
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl->renderer)
    {
        SDL_Log("Couldn't create renderer! %s\n", SDL_GetError());
        return false;
    }

    sdl->want = (SDL_AudioSpec){
        .freq = 44100,
        .format = AUDIO_S16SYS,
        .channels = 1,
        .callback = audio_callback,
        .samples = 512,
        .userdata = config,
    };
    sdl->dev = SDL_OpenAudioDevice(NULL, 0, &sdl->want, &sdl->have, 0);
    if (sdl->dev == 0)
    {
        SDL_Log("No audio device");
        return false;
    }
    if (sdl->want.channels != sdl->have.channels || sdl->want.format != sdl->have.format)
    {
        SDL_Log("Could not get desired audio channel");
        return false;
    }

    return true;
}

// SDL Cleanup Function
void final_cleanup(sdl_w *sdl)
{
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_CloseAudioDevice(sdl->dev);
    SDL_Quit();
}

// Clear Screen
void clear_screen(sdl_w *sdl, config_w *config)
{
    const uint8_t r = (config->bg_color >> 24) & 0XFF;
    const uint8_t g = (config->bg_color >> 16) & 0XFF;
    const uint8_t b = (config->bg_color >> 8) & 0XFF;
    const uint8_t a = (config->bg_color >> 0) & 0XFF;
    SDL_SetRenderDrawColor(sdl->renderer, r, g, b, a);
    SDL_RenderClear(sdl->renderer);
}

// Update Screen
void update_screen(sdl_w *sdl, chip8_w *chip8, config_w *config)
{
    SDL_Rect rect = {.x = 0, .y = 0, .w = config->scale_factor, .h = config->scale_factor};

    // const uint8_t fg_r = (config->fg_color >> 24) & 0x0FF;
    // const uint8_t fg_g = (config->fg_color >> 16) & 0x0FF;
    // const uint8_t fg_b = (config->fg_color >> 8) & 0x0FF;
    // const uint8_t fg_a = (config->fg_color >> 0) & 0x0FF;

    // const uint8_t bg_r = (config->bg_color >> 24) & 0XFF;
    // const uint8_t bg_g = (config->bg_color >> 16) & 0XFF;
    // const uint8_t bg_b = (config->bg_color >> 8) & 0XFF;
    // const uint8_t bg_a = (config->bg_color >> 0) & 0XFF;

    for (uint32_t i = 0; i < sizeof(chip8->display); i++)
    {
        if (chip8->display[i] == 1)
        {
            chip8->pixel_colors[i] = 1;
        }
        else
        {
            chip8->pixel_colors[i] = chip8->pixel_colors[i] - config->color_lerp_rate;
            if (chip8->pixel_colors[i] < 0.0f)
            {
                chip8->pixel_colors[i] = 0.0f;
            }
        }
    }
    // Loop through display pixels
    for (uint32_t i = 0; i < sizeof(chip8->display); i++)
    {
        rect.x = i % 64 * config->scale_factor;
        rect.y = i / 64 * config->scale_factor;

        float t = chip8->pixel_colors[i];
        uint32_t color = color_lerp(config->bg_color, config->fg_color, t);

        uint8_t r = (color >> 24) & 0xFF;
        uint8_t g = (color >> 16) & 0xFF;
        uint8_t b = (color >> 8) & 0xFF;
        uint8_t a = color & 0xFF;

        SDL_SetRenderDrawColor(sdl->renderer, r, g, b, a);
        SDL_RenderFillRect(sdl->renderer, &rect);
    }
    SDL_RenderPresent(sdl->renderer);
}

void update_timers(sdl_w sdl, chip8_w *chip8)
{
    if (chip8->delay_timer > 0)
        chip8->delay_timer--;
    if (chip8->sound_timer > 0)
    {
        chip8->sound_timer--;
        SDL_PauseAudioDevice(sdl.dev, 0);
    }
    else
    {
        SDL_PauseAudioDevice(sdl.dev, 1);
    }
}