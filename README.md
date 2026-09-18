# Chip-Eight

A lightweight, accurate CHIP-8 interpreter written in C using SDL2 for graphics, audio, and input handling.

![C](https://img.shields.io/badge/Language-C-blue)
![SDL2](https://img.shields.io/badge/Library-SDL2-green)
![Build](https://img.shields.io/badge/Build-CMake-orange)

---

## Features

* **Accurate CPU Emulation**: I made the full implementation of standard CHIP-8 instruction set architecture.
* **Precise Timing**: It runs smooth with precise timing.
* **SDL2 Frontend**: Used SDL2 frontend for this version, planning on a Godot one.
* **Pre-built Binary Included**: Included a Ready-to-run executable available in `bin/` for instant testing.

---

## Keybindings

It uses the standard QWERTY layout:

| CHIP-8 Keypad | Keyboard Mapping |
| :---: | :---: |
| `1` `2` `3` `C` | `1` `2` `3` `4` |
| `4` `5` `6` `D` | `Q` `W` `E` `R` |
| `7` `8` `9` `E` | `A` `S` `D` `F` |
| `A` `0` `B` `F` | `Z` `X` `C` `V` |

---

## Building from Source

### Prerequisites
* C Compiler (`gcc`, `clang` etc)
> I used gcc
* [CMake](https://cmake.org/) (3.10+)
* [Ninja](https://ninja-build.org/) build tool
* **SDL2 Development Library**: Placed directly inside the project root directory (`/SDL2`).
> [Here is the SDL2 Installation guide](https://wiki.libsdl.org/SDL2/Installation)

### Compilation Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/Rapiphane/CHIP-EIGHT_SDL.git
   cd CHIP-EIGHT_SDL/src
   ```
2. Generate Build Files using Ninja:
```bash
    cmake -B build -G Ninja
```

> Both Cmake and Ninja should be in your system path

3. Compile the project:
```bash
    cmake --build build
```
4. Run the Emulator:
```bash
    cd build
    ./chip8c <path-to-rom>
```
> You can also run the pre-compiled exe in /bin