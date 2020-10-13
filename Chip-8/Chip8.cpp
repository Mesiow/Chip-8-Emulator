#include "Chip8.h"

Chip8::Chip8()
{
	initialize();
}

void Chip8::initialize()
{
	PC_ = 0x200; //start of program
	I_ = 0x0000;
	SP_ = 15; //point to the top of stack, grow downward to add onto the stack

	memset(V_, 0x00, sizeof(V_));
	memset(stack_, 0x0000, sizeof(stack_));
	memset(ram_, 0x00, sizeof(ram_));

	delayTimer_ = 0x00;
	soundTimer_ = 0x00;
}

void Chip8::loadRom(const std::string& romFile)
{
	std::ifstream file(romFile, std::ios::binary | std::ios::ate); //open file at the end
	int size = 0;
	if (file.is_open())
	{
		size = file.tellg();
		byte* buffer = new byte[size];

		file.seekg(0, std::ios::beg); //seek back to beginning of file
		file.read((char*)buffer, size);
		file.close();

		for (int i = 0; i < size; i++)
			ram_[0x200 + i] = buffer[i]; //load into ram


		printRAM(size);
		delete[] buffer;
	}
	else {
		printf("Rom failed to load");
	}
}

void Chip8::printRAM(int size)
{
	for (int i = 0x200; i < 0x200 + size; i++) {
		printf("%02X", ram_[i]);
		printf("\n");
	}
}
