cmake_minimum_required(VERSION 3.16)
project(Chip8Emulator C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror)

set(SRC
    main.c
    chip8.c
    display.c
)

add_executable(chip8c ${SRC})


target_include_directories(chip8c PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2/SDL2-2.32.6/x86_64-w64-mingw32/include
)


target_link_directories(chip8c PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2/SDL2-2.32.6/x86_64-w64-mingw32/lib
)


target_link_libraries(chip8c PRIVATE 
    mingw32 
    SDL2main 
    SDL2
)