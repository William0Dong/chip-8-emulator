#include "chip8keyboard.h"
#include <assert.h>
static void chip8_keyboard_ensure_in_bounds(int key)
{
    assert(key >= 0 && key < CHIP8_TOTAL_KEYS);
}

// Setting the virtual keyboard array to keyboard_map
void chip8_keyboard_set_map(struct chip8_keyboard* keyboard, const char* map)
{
    keyboard->keyboard_map = map;
}

// If key matches something in the virtual keyboard return the index of the virtual key
int chip8_keyboard_map(struct chip8_keyboard* keyboard, char key)
{
    
    for (int i = 0; i < CHIP8_TOTAL_KEYS; i++)
    {
        if (keyboard->keyboard_map[i] == key)
        {
            return i;
        }
    }

    return -1;
}

// Setting the current key's value of being pushed down to true
void chip8_keyboard_down(struct chip8_keyboard* keyboard, int key)
{
    keyboard->keyboard[key] = true;
}

// Setting the current key's value of being pushed down to false
void chip8_keyboard_up(struct chip8_keyboard* keyboard, int key)
{
    keyboard->keyboard[key] = false;
}

// Checking to see if a key is pushed down
bool chip8_keyboard_is_down(struct chip8_keyboard* keyboard, int key)
{
    return keyboard->keyboard[key];
}