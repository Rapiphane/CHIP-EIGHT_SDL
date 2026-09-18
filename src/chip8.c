#include "chip8.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <SDL2/SDL.h>

// Initialise Chip8
bool init_chip8(chip8_w *chip8, const char rom[])
{
    const uint32_t entry_point = 0X200; // Entry Point
    const uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    memset(chip8, 0, sizeof(chip8_w));
    // Load Font
    memcpy(&chip8->ram[0], font, sizeof(font));

    // Load ROM
    FILE *rom_file = fopen(rom, "rb");
    if (!rom_file)
    {
        perror("fopen error");
        printf("Rom file %s does not load!\n", rom);
        return false;
    }
    fseek(rom_file, 0, SEEK_END);
    size_t rom_size = ftell(rom_file);
    size_t max_size = sizeof chip8->ram - entry_point;
    rewind(rom_file);

    if (rom_size > max_size)
    {
        printf("Rom size %s exceeds maximum limit!\n", rom);
        return false;
    }

    if (fread(&chip8->ram[entry_point], 1, rom_size, rom_file) != rom_size)
    {
        printf("Rom %s could not be read\n", rom);
        return false;
    }
    chip8->rom_name = rom;
    fclose(rom_file);
    // Set defaults

    chip8->state = RUNNING; // default
    chip8->PC = entry_point;
    chip8->SP = &chip8->stack[0];

    return true;
}

// Emulate CHIP8 Instruction
void emulate_instruction(chip8_w *chip8)
{
    // Get next Opcode
    chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | (chip8->ram[chip8->PC + 1]);
    chip8->PC += 2; // Increment PC by 2

    // Fill out instruction format
    chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
    chip8->inst.NN = chip8->inst.opcode & 0x0FF;
    chip8->inst.N = chip8->inst.opcode & 0x0F;
    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;

#ifdef DEBUG
    print_debug_info(chip8);
#endif
    // Emulate OPCode

    switch ((chip8->inst.opcode >> 12) & 0x0F)
    {
    case 0x0:
        if (chip8->inst.NN == 0xE0)
        {
            // clear screen
            memset(chip8->display, 0, sizeof(chip8->display));
            chip8->draw = true;
            printf("Clear Screen: 0x%04X\n", chip8->inst.opcode);
        }
        else if (chip8->inst.NN == 0xEE)
        {
            // Grab last address from stack and set it to PC
            chip8->SP--; // Decrease stack to get the last
            chip8->PC = *chip8->SP;
            printf("Return from subroutine: 0x%04X\n", chip8->inst.opcode);
        }
        break;
    case 0x1:
        chip8->PC = chip8->inst.NNN;
        printf("Set subroutine to NNN: 0x%04X\n", chip8->inst.opcode);
        break;

    case 0x2:
        // Call subroutine at NNN
        *chip8->SP = chip8->PC; // Store current address
        chip8->SP++;
        chip8->PC = chip8->inst.NNN;
        printf("Call subroutine at NNN: 0x%04X\n", chip8->inst.opcode);
        break;

    case 0x3:
        // Skips the next instruction if VX equals NN
        if (chip8->V[chip8->inst.X] == chip8->inst.NN)
        {
            chip8->PC += 2;
            printf("Skips the next instruction if V%X equals NN: 0x%04X\n", chip8->inst.X, chip8->inst.opcode);
        }
        break;

    case 0x4:
        // Skips the next instruction if VX does not equals NN
        if (chip8->V[chip8->inst.X] != chip8->inst.NN)
        {
            chip8->PC += 2;
            printf("Skips the next instruction if V%X  does not equals NN: 0x%04X\n", chip8->inst.X, chip8->inst.opcode);
        }
        break;

    case 0x5:
        // Skips the next instruction if VX equals VY
        if (chip8->V[chip8->inst.X] == chip8->V[chip8->inst.Y])
        {
            chip8->PC += 2;
            printf("Skips the next instruction if V%X  equals V%X: 0x%04X\n", chip8->inst.X, chip8->inst.Y, chip8->inst.opcode);
        }
        break;

    case 0x6:
        // Sets VX to NN
        chip8->V[chip8->inst.X] = chip8->inst.NN;
        printf("Set V%X to NN: 0x%04X\n", chip8->inst.X, chip8->inst.opcode);
        break;

    case 0x7:
        // Adds NN to VX
        chip8->V[chip8->inst.X] += chip8->inst.NN;
        printf("Add NN to V%X: 0x%04X\n", chip8->inst.X, chip8->inst.opcode);
        break;

    case 0x8:
        switch (chip8->inst.N)
        {
        case 0:
            // Sets VX to the value of VY
            chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y];
            printf("Sets V%X  to the value of V%X: 0x%04X\n", chip8->inst.X, chip8->inst.Y, chip8->inst.opcode);
            break;

        case 1:
            // Sets VX to VX or VY
            chip8->V[chip8->inst.X] |= chip8->V[chip8->inst.Y];
            chip8->V[0XF] = 0;
            printf("Sets V%X  to V%X or V%X: 0x%04X\n", chip8->inst.X, chip8->inst.X, chip8->inst.Y, chip8->inst.opcode);
            break;

        case 2:
            // Sets VX to VX and VY
            chip8->V[chip8->inst.X] &= chip8->V[chip8->inst.Y];
            chip8->V[0XF] = 0;
            printf("Sets V%X  to V%X and V%X: 0x%04X\n", chip8->inst.X, chip8->inst.X, chip8->inst.Y, chip8->inst.opcode);
            break;

        case 3:
            // Sets VX to VX xor VY
            chip8->V[chip8->inst.X] ^= chip8->V[chip8->inst.Y];
            chip8->V[0XF] = 0;
            printf("Sets V%X  to V%X xor V%X: 0x%04X\n", chip8->inst.X, chip8->inst.X, chip8->inst.Y, chip8->inst.opcode);
            break;

        case 4:
            // Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not.
            uint16_t sum = chip8->V[chip8->inst.X] + chip8->V[chip8->inst.Y];
            uint8_t carry = (sum > 255) ? 1 : 0;
            chip8->V[chip8->inst.X] = (uint8_t)(sum & 0xFF);
            chip8->V[0xF] = carry;

            printf("Adds V%X  to V%X: 0x%04X\n", chip8->inst.X, chip8->inst.Y, chip8->inst.opcode);
            break;

        case 5:
            // VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not.
            {
                uint8_t borrow_flag = (chip8->V[chip8->inst.X] >= chip8->V[chip8->inst.Y]) ? 1 : 0;

                chip8->V[chip8->inst.X] -= chip8->V[chip8->inst.Y];

                chip8->V[0xF] = borrow_flag;

                printf("V%X is subtracted from V%X: 0x%04X\n", chip8->inst.Y, chip8->inst.X, chip8->inst.opcode);
            }
            break;

        case 6:
            // Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF.
            {
                uint8_t y_val = chip8->V[chip8->inst.Y];
                uint8_t lsb = y_val & 0x01;

                chip8->V[chip8->inst.X] = y_val >> 1;
                chip8->V[0xF] = lsb;

                printf("Shifts V%X to the right by 1: 0x%04X\n", chip8->inst.X, chip8->inst.opcode);
            }
            break;

        case 7:
            // Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not.
            {
                uint8_t borrow_flag = (chip8->V[chip8->inst.Y] >= chip8->V[chip8->inst.X]) ? 1 : 0;
                chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y] - chip8->V[chip8->inst.X];
                chip8->V[0xF] = borrow_flag;
                printf("Sets V%X to V%X minus V%X: 0x%04X\n", chip8->inst.X, chip8->inst.Y, chip8->inst.X, chip8->inst.opcode);
            }
            break;

        case 0xE:
            // Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset.
            {
                uint8_t y_val = chip8->V[chip8->inst.Y];
                uint8_t msb = (y_val >> 7) & 0x01;
                chip8->V[chip8->inst.X] = y_val << 1;
                chip8->V[0xF] = msb;
                printf("Shifts V%X to the left by 1: 0x%04X\n", chip8->inst.X, chip8->inst.opcode);
            }
            break;

        default:
            printf("Unimplemented OPCode: 0x%04X\n", chip8->inst.opcode);
            break;
        }
        break;

    case 0x9:
        // Skips the next instruction if VX does not equal VY
        if (chip8->V[chip8->inst.X] != chip8->V[chip8->inst.Y])
        {
            chip8->PC += 2;
            printf("Skips the next instruction if V%X does not equals V%X: 0x%04X\n", chip8->inst.X, chip8->inst.Y, chip8->inst.opcode);
        }

        break;

    case 0xA:
        // Sets I to address NNN
        chip8->I = chip8->inst.NNN;
        printf("Set I to address NNN: 0x%04X\n", chip8->inst.opcode);
        break;

    case 0xB:
        // Jumps to the address NNN plus V0
        chip8->PC = chip8->inst.NNN + chip8->V[0];
        printf("Jumps to the address NNN plus V0: 0x%04X\n", chip8->inst.opcode);
        break;

    case 0xC:
        // Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN
        {
            uint8_t rand_num = rand() % 256;
            chip8->V[chip8->inst.X] = rand_num & chip8->inst.NN;
            printf("Sets V%X to the result of a bitwise and operation on a random number and NN: 0x%04X\n", chip8->inst.X, chip8->inst.opcode);
        }
        break;

    case 0xD:

        if (chip8->draw)
        {
            chip8->PC -= 2;
            return; // Exit cycle early and wait for next frame
        }
        // Draw sprite
        uint8_t X_coord = chip8->V[chip8->inst.X] % 64;
        uint8_t Y_coord = chip8->V[chip8->inst.Y] % 32;
        uint8_t orig_X = X_coord;
        chip8->V[0XF] = 0; // Initialise carry flag to 0

        printf("Drawing Sprite: 0x%4X\n", chip8->inst.X);
        for (uint8_t i = 0; i < chip8->inst.N; i++)
        {
            // Get next row of sprite data
            const uint8_t sprite_data = chip8->ram[chip8->I + i];
            X_coord = orig_X;

            for (int8_t j = 7; j >= 0; j--)
            {
                uint8_t pixel_x = (X_coord + (7 - j)) % 64;
                uint16_t draw_offset = (Y_coord % 32) * 64 + pixel_x;
                bool sprite_pixel = (sprite_data & (1 << j)) != 0;

                if (sprite_pixel)

                {
                    if (chip8->display[draw_offset])
                    {
                        chip8->V[0xF] = 1;
                    }
                    // XOR Display Pixel
                    chip8->display[draw_offset] ^= true;
                }
                if (pixel_x == 63)
                    break;
            }

            if (++Y_coord >= 32)
                break;
        }
        chip8->draw = true;
        break;

    case 0xE:
        if (chip8->inst.NN == 0x9E)
        {
            // Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed
            if (chip8->keypad[chip8->V[chip8->inst.X]])
            {
                chip8->PC += 2;
            }
        }
        else if (chip8->inst.NN == 0xA1)
        {
            // Skips the next instruction if the key stored in VX(only consider the lowest nibble) is not pressed
            if (!(chip8->keypad[chip8->V[chip8->inst.X]]))
            {
                chip8->PC += 2;
            }
        }
        break;

    case 0xF:
        switch (chip8->inst.NN)
        {
        case 0x0A: // FX0A - Wait for a key press, store key in VX
        {
            bool key_pressed = false;
            static int8_t last_key_down = -1;

            for (uint8_t i = 0; i < 16; i++)
            {
                if (chip8->keypad[i])
                {
                    last_key_down = i; // Store key index in VX
                    key_pressed = true;
                    break;
                }
            }

            if (!key_pressed && last_key_down != -1)
            {
                chip8->V[chip8->inst.X] = (uint8_t)last_key_down;
                last_key_down = -1;
            }
            else
            {
                chip8->PC -= 2; // Pause execution until release
            }

            printf("Wait for key press stored in V%X: 0x%04X\n", chip8->inst.X, chip8->inst.opcode);
        }
        break;

        case 0x1E:
            // Adds VX to I. VF is not affected.
            chip8->I += chip8->V[chip8->inst.X];
            break;

        case 0x07:
            // Sets VX to the value of the delay timer
            chip8->V[chip8->inst.X] = chip8->delay_timer;
            break;

        case 0x15:
            // Sets the delay timer to VX
            chip8->delay_timer = chip8->V[chip8->inst.X];
            break;

        case 0x18:
            // Sets the sound timer to VX
            chip8->sound_timer = chip8->V[chip8->inst.X];
            break;

        case 0x29:
            // Sets I to the location of the sprite for the character in VX (only consider the lowest nibble)
            chip8->I = chip8->V[chip8->inst.X] * 5;
            break;

        case 0x33:
            // Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2
            uint8_t number = chip8->V[chip8->inst.X];

            chip8->ram[chip8->I + 2] = number % 10;
            number /= 10;
            chip8->ram[chip8->I + 1] = number % 10;
            number /= 10;
            chip8->ram[chip8->I] = number;
            break;

        case 0x55:
            // Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
            // uint16_t offsetI = chip8->I;
            for (uint8_t i = 0; i <= chip8->inst.X; i++)
            {
                chip8->ram[chip8->I] = chip8->V[i];
                chip8->I++;
            }

            break;

        case 0x65:
            // Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified
            for (uint8_t i = 0; i <= chip8->inst.X; i++)
            {
                chip8->V[i] = chip8->ram[chip8->I];
                chip8->I++;
            }

            break;
        default:
            break;
        }
        break;

    default:
        printf("Unimplemented OPCode: 0x%04X\n", chip8->inst.opcode);
        break;
    }
}