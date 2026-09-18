#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "chip8.h"
#include "display.h"

// Handle Input
void handle_input(chip8_w *chip8, config_w *config)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            // Quit
            chip8->state = QUIT; // Exit
            return;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                // Exit Window
                chip8->state = QUIT;
                return;
            case SDLK_SPACE:
                if (chip8->state == RUNNING)
                {
                    chip8->state = PAUSED;
                    SDL_Log("The Game Is Paused");
                }
                else
                {
                    chip8->state = RUNNING;
                    SDL_Log("The Game Is Running");
                }
                return;

            case SDLK_j:
                if (config->color_lerp_rate > 0.1f)
                    config->color_lerp_rate -= 0.1;
                break;

            case SDLK_k:
                if (config->color_lerp_rate < 1.0f)
                    config->color_lerp_rate += 0.1;
                break;

            case SDLK_o:
                if (config->volume > 0)
                    config->volume -= 500;
                break;

            case SDLK_p:
                if (config->volume < INT16_MAX)
                    config->volume += 500;
                break;

            case SDLK_EQUALS: // Reset Chip8
                init_chip8(chip8, chip8->rom_name);
                break;
            case SDLK_1:
                chip8->keypad[0x1] = true;
                break;
            case SDLK_2:
                chip8->keypad[0x2] = true;
                break;
            case SDLK_3:
                chip8->keypad[0x3] = true;
                break;
            case SDLK_4:
                chip8->keypad[0xC] = true;
                break;

            case SDLK_q:
                chip8->keypad[0x4] = true;
                break;
            case SDLK_w:
                chip8->keypad[0x5] = true;
                break;
            case SDLK_e:
                chip8->keypad[0x6] = true;
                break;
            case SDLK_r:
                chip8->keypad[0xD] = true;
                break;

            case SDLK_a:
                chip8->keypad[0x7] = true;
                break;
            case SDLK_s:
                chip8->keypad[0x8] = true;
                break;
            case SDLK_d:
                chip8->keypad[0x9] = true;
                break;
            case SDLK_f:
                chip8->keypad[0xE] = true;
                break;

            case SDLK_z:
                chip8->keypad[0xA] = true;
                break;
            case SDLK_x:
                chip8->keypad[0x0] = true;
                break;
            case SDLK_c:
                chip8->keypad[0xB] = true;
                break;
            case SDLK_v:
                chip8->keypad[0xF] = true;
                break;

            default:
                break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym)
            {
            case SDLK_1:
                chip8->keypad[0x1] = false;
                break;
            case SDLK_2:
                chip8->keypad[0x2] = false;
                break;
            case SDLK_3:
                chip8->keypad[0x3] = false;
                break;
            case SDLK_4:
                chip8->keypad[0xC] = false;
                break;

            case SDLK_q:
                chip8->keypad[0x4] = false;
                break;
            case SDLK_w:
                chip8->keypad[0x5] = false;
                break;
            case SDLK_e:
                chip8->keypad[0x6] = false;
                break;
            case SDLK_r:
                chip8->keypad[0xD] = false;
                break;

            case SDLK_a:
                chip8->keypad[0x7] = false;
                break;
            case SDLK_s:
                chip8->keypad[0x8] = false;
                break;
            case SDLK_d:
                chip8->keypad[0x9] = false;
                break;
            case SDLK_f:
                chip8->keypad[0xE] = false;
                break;

            case SDLK_z:
                chip8->keypad[0xA] = false;
                break;
            case SDLK_x:
                chip8->keypad[0x0] = false;
                break;
            case SDLK_c:
                chip8->keypad[0xB] = false;
                break;
            case SDLK_v:
                chip8->keypad[0xF] = false;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    // Default Usage Message

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s<rom_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Initialise srand
    srand(time(NULL));

    // Initialise Config
    config_w config = {0};
    if (!set_initial_config(&config, argc, argv))
        exit(EXIT_FAILURE);

    // Initialise SDL
    sdl_w sdl = {0};
    if (!init_sdl(&sdl, &config))
        exit(EXIT_FAILURE);

    // Initialise Chip8
    chip8_w chip8 = {0};
    char *rom_name = argv[1];
    if (!init_chip8(&chip8, rom_name))
        exit(EXIT_FAILURE);

    // Initial Render Clear
    clear_screen(&sdl, &config);

    // Main Loop
    while (chip8.state != QUIT)
    {
        // Handle Input
        handle_input(&chip8, &config);

        if (chip8.state == PAUSED)
            continue;

        //  Get time
        uint64_t before_time = SDL_GetPerformanceCounter();
        //   Emulator instructions
        for (uint32_t i = 0; i < config.ips / 60; i++)
        {
            emulate_instruction(&chip8);
        }
        uint64_t after_time = SDL_GetPerformanceCounter();

        double time_elapsed = (double)((after_time - before_time) * 1000) / SDL_GetPerformanceFrequency();

        const double target_frame_time = 1000.0 / 60.0;
        //    Delay
        if (time_elapsed < target_frame_time)
        {
            SDL_Delay((uint32_t)(target_frame_time - time_elapsed));
        }
        // Update
        if (chip8.draw)
        {
            update_screen(&sdl, &chip8, &config);
            chip8.draw = false;
        }
        update_timers(sdl, &chip8);
    }

    // Final Cleanup
    final_cleanup(&sdl);

    exit(EXIT_SUCCESS);
    return 0;
}