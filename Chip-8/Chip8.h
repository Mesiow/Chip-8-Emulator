#pragma once
#include <stdio.h>
#include <string.h>
#include <string>
#include <cstdlib>
#include <time.h>
#include <fstream>
#include <SDL.h>

const int SAMPLE_RATE = 44100;
const int AMPLITUDE = 28000;

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
	~Chip8();
	void run();
	void checkInput(SDL_Event &ev);
	void loadRom(const std::string& romFile);
	void debugRender();

private:
	word fetchOpcode();
	void decodeAndExecute();
	void incrementPC(byte length);
	//execute opcodes under 0NNN
	void execute0NNN(word opcode);
	//execute opcodes 8XYN (N = 0 - E)
	void execute8XYN(word opcode);
	//execute FX opcodes
	void executeFX(word opcode);

	//System Initialization
	void initialize();
	//Audio setup
	void initializeAudio();
	static void audio_callback(void* userdata, byte* raw_buffer, int bytes);

	//debugging
	void printRAM(int size);

private:
	//Data Registers
	byte V_[16]; //V0 - VF, VF acts as a flag
	byte X, Y; //register indexes
	word stack_[16]; //up to 16 levels of nested subroutines
	//address register I used for storing memory addresses 
	//so only the lowest 12 bits are usually used
	word I_;
	word PC_; //program counter
	byte SP_; //stack pointer

	byte ram_[0x1000]; //4kb ram
public:
	std::uint32_t graphics[64 * 32]; //2kb graphics, monochrome screen
	byte keypad[16];
private:

	//timers
	byte delayTimer_;
	byte soundTimer_;
	SDL_AudioDeviceID device_;
};

