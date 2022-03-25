#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const unsigned int ADDR_INST_START = 0x200;
const unsigned int NUM_CHAR = 80;
const unsigned int ADDR_CHAR_START = 0x50;               

struct components {
    u_int8_t delayTimer;
    u_int8_t soundTimer;
    u_int16_t pc;
    u_int16_t stack[16];
    u_int8_t registers;
    u_int8_t memory[4096];
};
struct components chip8;

u_int8_t characters[NUM_CHAR] = {
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


void loadROM(char const* filename) {
    FILE* binary_ROM = fopen(filename, "rb");
    fseek(binary_ROM, 0, SEEK_END);
    int size = ftell(binary_ROM);
    char* buffer = malloc(size + 1);
    fread(buffer, size, 1, binary_ROM);
    rewind(binary_ROM);
    fclose(binary_ROM);
    for (long i = 0; i < size; ++i) {
        chip8.memory[ADDR_INST_START + i] = buffer[i];
    }
    free(buffer);
}

void Chip8Start() {
    chip8.pc = ADDR_INST_START;
    for (long i = 0; i < NUM_CHAR; ++i)
	{
		chip8.memory[ADDR_CHAR_START + i] = characters[i];
	}
}