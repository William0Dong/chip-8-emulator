#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const unsigned int ADDR_INST_START = 0x200;
const unsigned int NUM_CHAR = 80;
const unsigned int ADDR_CHAR_START = 0x50;       
const unsigned int VIDEO_WIDTH = 640;             
const unsigned int VIDEO_HEIGHT = 320;                 

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
	u_int16_t index;
	u_int8_t keypad[16];

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
    for (int i = 0; i < size; ++i) {
        chip8.memory[ADDR_INST_START + i] = buffer[i];
    }
    free(buffer);
}

void Chip8Start() {
    chip8.pc = ADDR_INST_START;
    for (int i = 0; i < NUM_CHAR; ++i)
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
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t byte = chip8.opcode & 0x00FFu;

	if (chip8.registers[Vx] == byte)
	{
		chip8.pc += 2;
	}
}

// Skips next instruction if VX does not equal KK (SNE Vx, byte)
void OP_4xkk()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t byte = chip8.opcode & 0x00FFu;

	if (chip8.registers[Vx] != byte)
	{
		chip8.pc += 2;
	}
}

// Skip next instruction if VX = VY (SE Vx, Vy)
void OP_5xy0()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t Vy = (chip8.opcode & 0x00F0) >> 4;

	if (chip8.registers[Vx] == chip8.registers[Vy])
	{
		chip8.pc += 2;
	}
}

// Set Vx = kk (LD Vx, byte)
void OP_6xkk()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t byte = chip8.opcode & 0x00FFu;
	chip8.registers[Vx] = byte;
}

// Set Vx = Vx + kk. (ADD Vx, byte)
void OP_7xkk()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t byte = chip8.opcode & 0x00FFu;

	chip8.registers[Vx] += byte;
}

// Set Vx = Vy (LD Vx, Vy)
void OP_8xy0()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t Vy = (chip8.opcode & 0x00F0) >> 4;

	chip8.registers[Vx] = chip8.registers[Vy];
}

// Set Vx = Vx OR Vy. (OR Vx, Vy)
void OP_8xy1()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t Vy = (chip8.opcode & 0x00F0) >> 4;

	chip8.registers[Vx] |= chip8.registers[Vy];
}

// Set Vx = Vx AND Vy. (AND Vx, Vy)
void OP_8xy2()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t Vy = (chip8.opcode & 0x00F0) >> 4;

	chip8.registers[Vx] &= chip8.registers[Vy];
}

// Set Vx = Vx XOR Vy. (XOR Vx, Vy)
void OP_8xy3()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t Vy = (chip8.opcode & 0x00F0) >> 4;

	chip8.registers[Vx] ^= chip8.registers[Vy];
}

// Set Vx = Vx + Vy, set VF = carry (ADD Vx, Vy)
void OP_8xy4()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t Vy = (chip8.opcode & 0x00F0) >> 4;

	u_int16_t sum = chip8.registers[Vx] + chip8.registers[Vy];

	if (sum > 0xFF)
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
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t Vy = (chip8.opcode & 0x00F0) >> 4;

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

// Set Vx = Vx SHR 1 (SHR Vx)
void OP_8xy6()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	// Save LSB in VF
	chip8.registers[0xF] = (chip8.registers[Vx] & 0x1u);

	chip8.registers[Vx] >>= 1;
}

// Set Vx = Vy - Vx, set VF = NOT borrow (SUBN Vx, Vy)
void OP_8xy7()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t Vy = (chip8.opcode & 0x00F0) >> 4;

	if (chip8.registers[Vy] > chip8.registers[Vx])
	{
		chip8.registers[0xF] = 1;
	}
	else
	{
		chip8.registers[0xF] = 0;
	}

	chip8.registers[Vx] = chip8.registers[Vy] - chip8.registers[Vx];
}

// Set Vx = Vx SHL 1 (SHL Vx {, Vy})
void OP_8xyE()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	chip8.registers[0xF] = (chip8.registers[Vx] & 0x80) >> 7u;

	chip8.registers[Vx] <<= 1;
}

// Skip next instruction if Vx != Vy
void OP_9xy0()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t Vy = (chip8.opcode & 0x00F0) >> 4;

	if (chip8.registers[Vx] != chip8.registers[Vy])
	{
		chip8.pc += 2;
	}
}

// Set I = nnn. (LD I, addr)
void OP_Annn()
{
	u_int16_t address = chip8.opcode & 0x0FFFu;

	chip8.index = address;
}

// Jump to location nnn + V0. (JP V0, addr)
void OP_Bnnn()
{
	u_int16_t address = chip8.opcode & 0x0FFFu;

	chip8.pc = chip8.registers[0] + address;
}

void OP_Cxkk()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t byte = chip8.opcode & 0x00FFu;
	chip8.registers[Vx] = randomByte() & byte;
}

void OP_Dxyn()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t Vy = (chip8.opcode & 0x00F0) >> 4;
	u_int8_t height = chip8.opcode & 0x000Fu;


	u_int8_t xPos = chip8.registers[Vx] % VIDEO_WIDTH;
	u_int8_t yPos = chip8.registers[Vy] % VIDEO_HEIGHT;

	chip8.registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		u_int8_t spriteByte = chip8.memory[chip8.index + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			u_int8_t spritePixel = spriteByte & (0x80 >> col);
			u_int32_t* screenPixel = &chip8.video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];


			if (spritePixel)
			{

				if (*screenPixel == 0xFFFFFFFF)
				{
					chip8.registers[0xF] = 1;
				}

				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

// Skip next instruction if key with the value of Vx is pressed.
void OP_Ex9E()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	u_int8_t key = chip8.registers[Vx];

	if (chip8.keypad[key])
	{
		chip8.pc += 2;
	}
}

// Skip next instruction if key with the value of Vx is not pressed.
void OP_ExA1()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	u_int8_t key = chip8.registers[Vx];

	if (!chip8.keypad[key])
	{
		chip8.pc += 2;
	}
}

// Skip next instruction if key with the value of Vx is not pressed (SKNP Vx)
void OP_ExA1()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	u_int8_t key = chip8.registers[Vx];

	if (!chip8.keypad[key])
	{
		chip8.pc += 2;
	}
}

// Set Vx = delay timer value.
void OP_Fx07()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	chip8.registers[Vx] = chip8.delayTimer;
}

// Wait for a key press, store the value of the key in Vx (Fx0A - LD Vx, K)
void OP_Fx0A()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	if (chip8.keypad[0])
	{
		chip8.registers[Vx] = 0;
	}
	else if (chip8.keypad[1])
	{
		chip8.registers[Vx] = 1;
	}
	else if (chip8.keypad[2])
	{
		chip8.registers[Vx] = 2;
	}
	else if (chip8.keypad[3])
	{
		chip8.registers[Vx] = 3;
	}
	else if (chip8.keypad[4])
	{
		chip8.registers[Vx] = 4;
	}
	else if (chip8.keypad[5])
	{
		chip8.registers[Vx] = 5;
	}
	else if (chip8.keypad[6])
	{
		chip8.registers[Vx] = 6;
	}
	else if (chip8.keypad[7])
	{
		chip8.registers[Vx] = 7;
	}
	else if (chip8.keypad[8])
	{
		chip8.registers[Vx] = 8;
	}
	else if (chip8.keypad[9])
	{
		chip8.registers[Vx] = 9;
	}
	else if (chip8.keypad[10])
	{
		chip8.registers[Vx] = 10;
	}
	else if (chip8.keypad[11])
	{
		chip8.registers[Vx] = 11;
	}
	else if (chip8.keypad[12])
	{
		chip8.registers[Vx] = 12;
	}
	else if (chip8.keypad[13])
	{
		chip8.registers[Vx] = 13;
	}
	else if (chip8.keypad[14])
	{
		chip8.registers[Vx] = 14;
	}
	else if (chip8.keypad[15])
	{
		chip8.registers[Vx] = 15;
	}
	else
	{
		chip8.pc -= 2;
	}
}

// Set delay timer = Vx (LD DT, Vx)
void OP_Fx15()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	chip8.delayTimer = chip8.registers[Vx];
}

// Set sound timer = Vx (LD ST, Vx)
void OP_Fx18()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	chip8.soundTimer = chip8.registers[Vx];
}

// Set I = I + Vx (ADD I, Vx)
void OP_Fx1E()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	chip8.index += chip8.registers[Vx];
}

// Set I = location of sprite for digit Vx (LD F, Vx)
void OP_Fx29()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t digit = chip8.registers[Vx];

	chip8.index = ADDR_CHAR_START + (5 * digit);
}

// Store BCD representation of Vx in memory locations I, I+1, and I+2 (LD B, Vx)
void OP_Fx33()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;
	u_int8_t value = chip8.registers[Vx];

	chip8.memory[chip8.index + 2] = value % 10;
	value /= 10;

	chip8.memory[chip8.index + 1] = value % 10;
	value /= 10;

	chip8.memory[chip8.index] = value % 10;
}

// Store registers V0 through Vx in memory starting at location I (LD [I], Vx)
void OP_Fx55()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	for (u_int8_t i = 0; i <= Vx; ++i)
	{
		chip8.memory[chip8.index + i] = chip8.registers[i];
	}
}

// Read registers V0 through Vx from memory starting at location I (LD Vx, [I])
void OP_Fx65()
{
	u_int8_t Vx = (chip8.opcode & 0x0F00) >> 8;

	for (u_int8_t i = 0; i <= Vx; ++i)
	{
		chip8.registers[i] = chip8.memory[chip8.index + i];
	}
}

void Cycle() {

	chip8.opcode = (chip8.memory[chip8.pc] << 8u) | chip8.memory[chip8.pc + 1];


	if (chip8.delayTimer > 0)
	{
		--chip8.delayTimer;
	}

	if (chip8.soundTimer > 0)
	{
		--chip8.soundTimer;
	}
}


void Video() {
	
}

void Input() {

}

int main() {
	Video();
	Input();
	Chip8Start();
	loadRom();
	while (!exit) {
		Cycle();
		KeyState();
	}
}