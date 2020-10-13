#include "Chip8.h"

Chip8::Chip8()
{
	initialize();
}



void Chip8::execute8XYN()
{
}

word Chip8::fetchOpcode()
{
	//Instruction is stored big endian
	//hi, lo
	word opcode = (ram_[PC_] << 8) | ram_[PC_ + 1];
	return opcode;
}

void Chip8::decodeAndExecute()
{
	word opcode = fetchOpcode();
	
	switch (opcode & 0xF000) { //tells us which instruction 1 - F
		case 0x0000: { //0NNN
			execute0NNN(opcode);
		}
		break;

		case 0x1000: { //1NNN
			PC_ = opcode & 0xFFF;
		}
		break;

		case 0x2000:{ //2NNN - call subroutine at nnn
			SP_ -= 1;
			stack_[SP_] = PC_;
			PC_ = opcode & 0xFFF;
		}
		break;

		case 0x3000: { //3XNN - skip following instruction if the value of register VX equals NN
			if (V_[(opcode & 0x0F00) >> 8] == (opcode & 0xFF))
				incrementPC(4); //skip follwing instr
			else
				incrementPC(2); //go to following instr
		}
		break;
	}
}

void Chip8::incrementPC(byte length)
{
	PC_ += length;
}

void Chip8::execute0NNN(word instruction)
{
	switch (instruction) {
		//sys addr, ignored by modern systems
	case 0x00E0:
		memset(graphics_, 0x00, sizeof(graphics_));
		incrementPC(2);
		break;
	case 0x00EE: //return from subroutine
		PC_ = stack_[SP_];
		SP_ += 1; //decrease stack size
		break;
	}
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


		//printRAM(size);
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
