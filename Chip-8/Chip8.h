#pragma once
#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include <SDL.h>

using byte = unsigned char;
using word = unsigned short;

//Each Chip8 instruction is 2 bytes long, stored in
//big endian fashion

//Chip 8 programs should we loaded into memory starting at
//0x200

/*

	Memory Map:
+---------------+= 0xFFF (4095) End of Chip-8 RAM
|               |
|               |
|               |
|               |
|               |
| 0x200 to 0xFFF|
|     Chip-8    |
| Program / Data|
|     Space     |
|               |
|               |
|               |
+- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
|               |
|               |
|               |
+---------------+= 0x200 (512) Start of most Chip-8 programs
| 0x000 to 0x1FF|
| Reserved for  |
|  interpreter  |
+---------------+= 0x000 (0) Start of Chip-8 RAM

*/

class Chip8 {
public:
	Chip8();
	void initialize();
	void loadRom(const std::string& romFile);

private:
	void printRAM(int size);

private:
	//Data Registers
	byte V_[16]; //V0 - VF, VF acts as a flag
	word stack_[16]; //up to 16 levels of nested subroutines
	//address register I used for storing memory addresses 
	//so only the lowest 12 bits are usually used
	word I_;
	word PC_; //program counter
	byte SP_; //stack pointer

	byte ram_[0x1000]; //4kb ram


	//timers
	byte delayTimer_;
	byte soundTimer_;
};