#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

// Emulator State
typedef enum
{
    QUIT,
    RUNNING,
    PAUSED
} emustate_w;

// Chip8 Instruction Format
typedef struct
{
    uint16_t opcode;
    uint16_t NNN; // 12 bit constant
    uint8_t NN;   // 8 bit
    uint8_t N;    // 4 bit
    uint8_t X;    // Register Identifier
    uint8_t Y;

} instruction_w;

// Chip8 Machine
typedef struct
{
    emustate_w state;
    uint8_t ram[4096];
    bool display[64 * 32]; // Emulate original res
    float pixel_colors[64 * 32];
    uint16_t stack[12];
    uint16_t *SP;
    uint8_t V[16]; // Data Registers
    uint16_t I;    // Index Register
    uint8_t delay_timer;
    uint8_t sound_timer;
    bool keypad[16];      // Hexadecimal Keypad
    uint16_t PC;          // Program Counter
    const char *rom_name; // Current ROM
    instruction_w inst;   // Execute Instruction
    bool draw;
} chip8_w;

bool init_chip8(chip8_w *chip8, const char rom[]);

void emulate_instruction(chip8_w *chip8);

#endif