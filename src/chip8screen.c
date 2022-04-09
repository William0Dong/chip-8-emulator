#include "chip8screen.h"
#include <assert.h>
#include <memory.h>


static void chip8_screen_check_bounds(int x, int y)
{
    assert(x >= 0 && x < CHIP8_WIDTH && y >= 0 && y < CHIP8_HEIGHT);
}

// Initializing pixel to a certain coordinate
void chip8_screen_set(struct chip8_screen* screen, int x, int y)
{
    chip8_screen_check_bounds(x, y);
    screen->pixels[y][x] = true;
}

// Clearing screen of pixels
void chip8_screen_clear(struct chip8_screen* screen)
{
    memset(screen->pixels, 0, sizeof(screen->pixels));
}

// Checking if there is a pixel on a certain coordinate
bool chip8_screen_is_set(struct chip8_screen* screen, int x, int y)
{
    chip8_screen_check_bounds(x, y);
    return screen->pixels[y][x];
}

// Drawing sprite to the virtual screen
bool chip8_screen_draw_sprite(struct chip8_screen* screen, int x, int y, const char* sprite, int num)
{   
    bool pixel_collison = false;

    // Using loops to get each pixel of the maximum 8x15 sprite where num helps us loop through the rows and there are eight colums per row
    for (int ly = 0; ly < num; ly++)
    {
        // Setting c to be the row of pixels
        char c = sprite[ly];
        for (int lx = 0; lx < 8; lx++)
        {
            // Checking to see if each pixel actually exists and if it doesnt skip to the next pixel of the row
            if ((c & (0b10000000 >> lx)) == 0) {
                continue;
            } 
            // Using modulus operator to determine the cooridnate of the pixel if it wraps to the other side of the screen

            // Checking to see if a pixel already exists at a pixel the sprite is to be drawn
            if (screen->pixels[(ly+y) % CHIP8_HEIGHT][(lx+x) % CHIP8_WIDTH])
            {
                pixel_collison = true;
            }
            // Pixels are XOR'd onto the screen
            screen->pixels[(ly+y) % CHIP8_HEIGHT][(lx+x) % CHIP8_WIDTH] ^= true;
        }
    }
    return pixel_collison;
}