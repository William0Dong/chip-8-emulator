#ifndef CHIP8REGISTERS_H
#define CHIP8REGISTERS_H

#include "config.h"

// Declaring all registers
// V[] are 16 bit general purpose 8 bit registers where VF is used as a flag for some opcodes
// PC is 16 bits and stores currently executing address
// SP is 8 bits and points to the topmost level of stack
// I is 16 bits but stores memory addresses so only 12 bits are usually used
// Sound timer stops making noise when it hits 0
// Delay timer deactivates when it hits 0
struct chip8_registers 
{
    unsigned char V[CHIP8_TOTAL_DATA_REGISTERS];
    unsigned short I;
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short PC;
    unsigned short SP;
};

#endif