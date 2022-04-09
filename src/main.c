#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include "SDL2/SDL.h"
#include "chip8.h"
#include "chip8keyboard.h"

// Virtual keyboard keys
const char keyboard_map[CHIP8_TOTAL_KEYS] = {
    SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5,
    SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_a, SDLK_b,
    SDLK_c, SDLK_d, SDLK_e, SDLK_f};

int main(int argc, char **argv)
{
    // Checking if a file was provided
    if (argc < 2)
    {
        printf("You must provide a file to load\n");
        return -1;
    }

    const char* filename = argv[1];
    printf("The filename to load is: %s\n", filename);

    FILE* f = fopen(filename, "rb");
    if (!f)
    {
        printf("Failed to open the file");
        return -1;
    }

    // Using position of end of file to determine the size and then moving the pointer back to the beginning
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Declaring a buffer
    char buf[size];
    int res = fread(buf, size, 1, f);
    if (res != 1)
    {
        printf("Failed to read from file");
        return -1;
    }
    
    // Initializing the chip8 virtual machine and its memory along with keyboard
    struct chip8 chip8;
    chip8_init(&chip8);
    chip8_load(&chip8, buf, size);
    chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);
   
    // Initializng SDL
    SDL_Init(SDL_INIT_EVERYTHING);

    // Creating a window in the middle of a screen with a dimension of 640 x 320 pixels
    SDL_Window *window = SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        CHIP8_WIDTH * CHIP8_WINDOW_MULTIPLIER,
        CHIP8_HEIGHT * CHIP8_WINDOW_MULTIPLIER, SDL_WINDOW_SHOWN);

    // Initializing renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);

    while (1)
    {
        // Checking if keyboard is pushed down or released and whether it matches to a virtual key to trigger functions
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                goto out;
                break;

            case SDL_KEYDOWN:
            {
                char key = event.key.keysym.sym;
                int vkey = chip8_keyboard_map(&chip8.keyboard, key);
                if (vkey != -1)
                {
                    chip8_keyboard_down(&chip8.keyboard, vkey);
                }
            }
            break;

            case SDL_KEYUP:
            {
                char key = event.key.keysym.sym;
                int vkey = chip8_keyboard_map(&chip8.keyboard, key);
                if (vkey != -1)
                {
                    chip8_keyboard_up(&chip8.keyboard, vkey);
                }
            }
            break;
            };
        }

        // Drawing the black background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        // Drawing white pixels in the foreground
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

        // Drawing virtual sprite to the actual SDL window
        for (int x = 0; x < CHIP8_WIDTH; x++)
        {
            for (int y = 0; y < CHIP8_HEIGHT; y++)
            {
                if (chip8_screen_is_set(&chip8.screen, x, y))
                {
                    // Drawing a sprite outline and filling it based on whether there are pixels already there
                    SDL_Rect r;
                    r.x = x * CHIP8_WINDOW_MULTIPLIER;
                    r.y = y * CHIP8_WINDOW_MULTIPLIER;
                    r.w = CHIP8_WINDOW_MULTIPLIER;
                    r.h = CHIP8_WINDOW_MULTIPLIER;
                    SDL_RenderFillRect(renderer, &r);
                }
            }
        }

        // Decrementing delay timer at 60hz
        SDL_RenderPresent(renderer);
        if (chip8.registers.delay_timer > 0) {
            Sleep(16.67);
            chip8.registers.delay_timer -= 1;

        }
        // Decrementing sound timer at 60hz
        if (chip8.registers.sound_timer > 0) {
            Beep(800, 1000 * (chip8.registers.sound_timer / 60.0));
            chip8.registers.sound_timer = 0;
        }
        
        // Getting 2 byte opcode and executing it
        unsigned short opcode = chip8_memory_get_short(&chip8.memory, chip8.registers.PC);
        chip8.registers.PC += 2;
        chip8_exec(&chip8, opcode);
    }

// Exiting emulator when close button is clicked
out:
    SDL_DestroyWindow(window);
    return 0;
}