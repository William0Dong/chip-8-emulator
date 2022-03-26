#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>  

const unsigned int ADDR_INST_START = 0x200;
const unsigned int NUM_CHAR = 80;
const unsigned int ADDR_CHAR_START = 0x50;               

struct components {
    u_int8_t delayTimer;
    u_int8_t soundTimer;
    u_int16_t pc;
    u_int16_t stack[16];
    u_int8_t registers[16];
    u_int8_t memory[4096];
    u_int32_t video[64 * 32];
    u_int8_t sp;
    u_int16_t opcode;
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
    FILE* binaryROM = fopen(filename, "rb");
    fseek(binaryROM, 0, SEEK_END);
    int size = ftell(binaryROM);
    char* buffer = malloc(size + 1);
    fread(buffer, size, 1, binaryROM);
    rewind(binaryROM);
    fclose(binaryROM);
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

int randomByte() {
    srand(time(0));
    int randomNumber = rand() % (255 + 1);
}

// CLEAR DISPLAY (CLS)
void OP_00E0() {
    memset(chip8.video, 0, sizeof(chip8.video));
}

// RETURN FROM SUBROUTINE (RET)
void OP_00EE(){
    --chip8.sp;
    chip8.pc = chip8.stack[chip8.sp];
}

// Jump to location (JP addr)
void OP_1nnn()
{
	u_int16_t address = chip8.opcode & 0x0FFFu;

	chip8.pc = address;
}

// Call subroutine (CALL addr)
void OP_2nnn() {
    u_int16_t address = chip8.opcode & 0x0FFFu;
	chip8.stack[chip8.sp] = chip8.pc;
	++chip8.sp;
	chip8.pc = address;
}

// Skip next instruction if VX = KK (SE Vx, byte)
void OP_3xkk() {
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t byte = chip8.opcode & 0x00FFu;

	if (chip8.registers[Vx] == byte)
	{
		chip8.pc += 2;
	}
}

// Skips next instruction if VX does not equal KK (SNE Vx, byte)
void OP_4xkk()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t byte = chip8.opcode & 0x00FFu;

	if (chip8.registers[Vx] != byte)
	{
		chip8.pc += 2;
	}
}

// Skip next instruction if VX = VY (SE Vx, Vy)
void OP_5xy0()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t Vy = (chip8.opcode & 0x00F0u) >> 4u;

	if (chip8.registers[Vx] == chip8.registers[Vy])
	{
		chip8.pc += 2;
	}
}

// Set Vx = kk (LD Vx, byte)
void OP_6xkk()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t byte = chip8.opcode & 0x00FFu;
	chip8.registers[Vx] = byte;
}

// Set Vx = Vx + kk. (ADD Vx, byte)
void OP_7xkk()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t byte = chip8.opcode & 0x00FFu;

	chip8.registers[Vx] += byte;
}

// Set Vx = Vy (LD Vx, Vy)
void OP_8xy0()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t Vy = (chip8.opcode & 0x00F0u) >> 4u;

	chip8.registers[Vx] = chip8.registers[Vy];
}

// Set Vx = Vx OR Vy. (OR Vx, Vy)
void OP_8xy1()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t Vy = (chip8.opcode & 0x00F0u) >> 4u;

	chip8.registers[Vx] |= chip8.registers[Vy];
}

// Set Vx = Vx AND Vy. (AND Vx, Vy)
void OP_8xy2()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t Vy = (chip8.opcode & 0x00F0u) >> 4u;

	chip8.registers[Vx] &= chip8.registers[Vy];
}

// Set Vx = Vx XOR Vy. (XOR Vx, Vy)
void OP_8xy3()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t Vy = (chip8.opcode & 0x00F0u) >> 4u;

	chip8.registers[Vx] ^= chip8.registers[Vy];
}

// Set Vx = Vx + Vy, set VF = carry (ADD Vx, Vy)
void OP_8xy4()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t Vy = (chip8.opcode & 0x00F0u) >> 4u;

	u_int16_t sum = chip8.registers[Vx] + chip8.registers[Vy];

	if (sum > 255U)
	{
		chip8.registers[0xF] = 1;
	}
	else
	{
		chip8.registers[0xF] = 0;
	}

	chip8.registers[Vx] = sum & 0xFFu;
}

// Set Vx = Vx - Vy, set VF = NOT borrow. (SUB Vx, Vy)
void OP_8xy5()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00u) >> 8u;
	u_int8_t Vy = (chip8.opcode & 0x00F0u) >> 4u;

	if (chip8.registers[Vx] > chip8.registers[Vy])
	{
		chip8.registers[0xF] = 1;
	}
	else
	{
		chip8.registers[0xF] = 0;
	}
	chip8.registers[Vx] -= chip8.registers[Vy];
}