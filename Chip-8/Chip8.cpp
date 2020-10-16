#include "Chip8.h"
#include <iostream>

double wave_position = 0;
double wave_increment = 2 * PI * 1000 / SAMPLE_RATE;


Chip8::Chip8()
{
	initialize();
}

Chip8::~Chip8()
{
	delete audiospec_;
	SDL_CloseAudioDevice(device_);
}

void Chip8::run()
{
	decodeAndExecute();
}

void Chip8::checkInput(SDL_Event &ev)
{
	switch (ev.type) {
		//keydown
	case SDL_KEYDOWN: {
		switch (ev.key.keysym.sym) {
		case SDLK_x: keypad[0] = 1; break; case SDLK_1: keypad[1] = 1; break;
		case SDLK_2: keypad[2] = 1; break; case SDLK_3: keypad[3] = 1; break;
		case SDLK_q: keypad[4] = 1; break; case SDLK_w: keypad[5] = 1; break;
		case SDLK_e: keypad[6] = 1; break; case SDLK_a: keypad[7] = 1; break;
		case SDLK_s: keypad[8] = 1; break; case SDLK_d: keypad[9] = 1; break;
		case SDLK_z: keypad[0xA] = 1; break; case SDLK_c: keypad[0xB] = 1; break;
		case SDLK_4: keypad[0xC] = 1; break; case SDLK_r:keypad[0xD] = 1; break;
		case SDLK_f: keypad[0xE] = 1; break; case SDLK_v: keypad[0xF] = 1; break;
		}
	}
					break;

		//keyup
	case SDL_KEYUP: {
		switch (ev.key.keysym.sym) {
		case SDLK_x: keypad[0] = 0; break; case SDLK_1: keypad[1] = 0; break;
		case SDLK_2: keypad[2] = 0; break; case SDLK_3: keypad[3] = 0; break;
		case SDLK_q: keypad[4] = 0; break; case SDLK_w: keypad[5] = 0; break;
		case SDLK_e: keypad[6] = 0; break; case SDLK_a: keypad[7] = 0; break;
		case SDLK_s: keypad[8] = 0; break; case SDLK_d: keypad[9] = 0; break;
		case SDLK_z: keypad[0xA] = 0; break; case SDLK_c:keypad[0xB] = 0; break;
		case SDLK_4: keypad[0xC] = 0; break; case SDLK_r:keypad[0xD] = 0; break;
		case SDLK_f: keypad[0xE] = 0; break; case SDLK_v:keypad[0xF] = 0; break;
		}
		break;
	}
	}
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
	
	//index values into V data register array
	X = (opcode & 0x0F00) >> 8;
	Y = (opcode & 0x00F0) >> 4;

	switch (opcode & 0xF000) { //tells us which instruction 1 - F
	case 0x0000: { //0NNN
		execute0NNN(opcode);
	}
			   break;

	case 0x1000: { //1NNN
		PC_ = (opcode & 0xFFF);
	}
			   break;

	case 0x2000: { //2NNN - call subroutine at nnn
		stack_[SP_] = PC_;
		SP_ -= 1;
		PC_ = opcode & 0xFFF;
	}
			   break;

	case 0x3000: { //3XNN - skip following instruction if the value of register VX equals NN
		byte NN = (opcode & 0xFF);
		if (V_[X] == NN)
			incrementPC(4); //skip follwing instr
		else
			incrementPC(2); //go to following instr
	}
			   break;

	case 0x4000: { //4XNN - skip following instruction if the val of reg VX is not equal to NN
		byte NN = (opcode & 0xFF);
		if (V_[X] != NN)
			incrementPC(4);
		else
			incrementPC(2);
	}
			   break;

	case 0x5000: { //5XY0 - skip following instruction if the value of register VX is equal to the value of register VY
		if (V_[X] == V_[Y])
			incrementPC(4);
		else
			incrementPC(2);
	}
			   break;

	case 0x6000: { //6XNN - store number NN in register VX
		byte NN = (opcode & 0xFF);
		V_[X] = NN;
		incrementPC(2);
	}
			   break;

	case 0x7000: { //7XNN - add the value NN to register VX
		byte NN = (opcode & 0xFF);
		V_[X] += NN;
		incrementPC(2);
	}
			   break;

	case 0x8000: { //execute 8XY(0 - E) opcodes
		execute8XYN(opcode);
	}
			   break;

	case 0x9000: { //9XY0 - skip the following instruction if the val of reg VX is not equal to val of reg VY
		if (V_[X] != V_[Y]) //skip
			incrementPC(4);
		else
			incrementPC(2);
	}
			   break;

	case 0xA000: { //ANNN - store mem address NNN in register I
		word NNN = (opcode & 0x0FFF);
		I_ = NNN;
		incrementPC(2);
	}
			   break;

	case 0xB000: { //BNNN - jump to address NNN + V0
		word NNN = (opcode & 0x0FFF);
		PC_ = NNN + V_[0];
	}
			   break;

	case 0xC000: { //CXNN - set VX to a random number with a mask of NN
		byte number = rand() % 255 + 0;
		number &= (opcode & 0xFF);
		V_[X] = number;
		incrementPC(2);
	}
			   break;

	//DXYN - Draw a sprite at position VX, VY with N bytes of sprite data
	//starting at the address stored in I.
	//Set VF to 01 if any set pixels are changed to unset, and 
	//Set VF to 00 otherwise.
	case 0xD000: {
		byte height = (opcode & 0x000F);

		V_[0xF] = 0x0;
		for (int i = 0; i < height; i++) {
			byte spriteByte = ram_[I_ + i]; //start read from mem I_

			for (int j = 0; j < 8; j++) { //loop width
				//get msb
				byte spritePixel = spriteByte & (0x80 >> j);
				//grab pixel from graphics memory location
				byte posX = V_[X];
				byte posY = V_[Y];
				std::uint32_t* screenPixel = &graphics[(posX + j + ((posY + i) * 64)) % (64 * 32)]; // % (64 * 32) to wrap if off screen
	

				//if sprite pixel is on
				if (spritePixel) {
					
					//screen pixel is also on, which signifies collision
					if (*screenPixel == 0xFFFFFFFF) {
						V_[0xF] = 0x1;
					}
					//xor with sprite pixel
					*screenPixel ^= pixelColor_.green;
					
				}
			}
		}
		incrementPC(2);
	}
			   break;

	case 0xE000: {
		switch (opcode & 0x00FF) {
		//EX9E - skip the following instruction if the key corresponding
		//to the hex value currently stored in register VX is pressed
		case 0x009E: {
			byte value = V_[X]; 
			if (keypad[value] != 0)
				incrementPC(4);
			else
				incrementPC(2);
		}
				   break;

		case 0x00A1: {
			byte value = V_[X];
			if (keypad[value] == 0) //skip next instruction
				incrementPC(4);
			else
				incrementPC(2);
		}
				   break;
		}
	}
			   break;

	case 0xF000: {
		executeFX(opcode);
	}
			   break;

	default:
		printf("\nUnknown op code: %.4X\n", opcode);
		break;
	}

	//update timers
	if (delayTimer_ > 0)
		--delayTimer_;

	if (soundTimer_ > 0) {
		if (soundTimer_ == 1)
		{
			//play sound
			SDL_PauseAudioDevice(device_, 0); //start playing sound
			SDL_Delay(100); //wait while sound is playing
			SDL_PauseAudioDevice(device_, 1); //stop playing sound
		}

		--soundTimer_;
	}
}

void Chip8::incrementPC(byte length)
{
	PC_ += length;
}

void Chip8::execute0NNN(word opcode)
{
	switch (opcode & 0x00FF) {
		//sys addr, ignored by modern systems
	case 0x00E0:
		memset(graphics, 0x00, sizeof(graphics));
		incrementPC(2);
		break;
	case 0x00EE: //return from subroutine
		SP_ += 1; //decrease stack size
		PC_ = stack_[SP_];
		incrementPC(2);
		break;
	}
}

void Chip8::execute8XYN(word opcode)
{
	switch (opcode & 0x000F) {
		case 0x0000: { //8XY0 - store value of register VY into VX
			V_[X] = V_[Y];
			incrementPC(2);
		}
		break;

		case 0x0001: { //8XY1 - set VX = VX bitwise OR VY
			V_[X] |= V_[Y];
			incrementPC(2);
		}
		break;

		case 0x0002: { //8XY2 - set VX = VX bitwise AND VY
			V_[X] &= V_[Y];
			incrementPC(2);
		}
		break;

		case 0x0003: { //8XY3 - set VX to VX bitwise XOR VY
			V_[X] ^= V_[Y];
			incrementPC(2);
		}
		break;

		//8XY4 - add the value of reg VY to register VX. 
		//Set VF(V_[15]) to 01 if a carry occurs
		//Set VF to 00 if a carry does not occur
		case 0x0004: {
			word result = V_[X] + V_[Y];
			if (result > 0xFF) //carry occured
				V_[0xF] = 0x1;
			else
				V_[0xF] = 0x0;

			V_[X] = result & 0xFF;
			incrementPC(2);
		}
		break;

		//8XY5 - subtract the value of VY from reg VX.
		//Set VF to 00 if a borrow occurs
		//Set VF to 01 if a borrow does not occur
		case 0x0005: {
			if (V_[Y] > V_[X]) //borrow occurs
				V_[0xF] = 0x0;
			else
				V_[0xF] = 0x1;

			V_[X] -= V_[Y];
			incrementPC(2);
		}
		break;

		//8XY6 - store the value of register VY shifted right one bit into register VX
		//Set register VF to the lsb prior to the shift
		//VY is unchanged
		case 0x0006: {
			byte value = V_[Y] >> 1;
			V_[0xF] = V_[Y] & 0x1;
			V_[X] = value;
			incrementPC(2);
		}
		break;

		//8XY7 - set register VX to the value of VY minus VX
		//Set VF to 00 if a borrow occurs
		//Set VF to 01 if a borrow does not occur
		case 0x0007: {
			if (V_[X] > V_[Y]) //borrow occurs
				V_[0xF] = 0x0;
			else
				V_[0xF] = 0x1;

			V_[X] = V_[Y] - V_[X];
			incrementPC(2);
		}
		break;

		//8XYE - store the value of register VY shifted left one bit into register VX
		//Set register VF to the msb prior to the shift
		//VY is unchanged
		case 0x000E: {
			byte value = V_[Y] << 1;
			//if VY is a nibble value msb location is 0x8
			//else VY is a byte msb location is 80
			if (V_[Y] <= 0xF)
				V_[0xF] = (V_[Y] & 0x8) >> 3;
			else
				V_[0xF] = (V_[Y] & 0x80) >> 7;

			V_[X] = value;
			incrementPC(2);
		}
		break;
	}
}

void Chip8::executeFX(word opcode)
{
	switch (opcode & 0x00FF) {
	//FX07 - store the current value of the delay timer in reg VX
	case 0x0007: {
		V_[X] = delayTimer_;
		incrementPC(2);
	}
			   break;

	//FX0A - wait for a keypress and store the result in reg VX
	case 0x000A: {
		bool keypressed = false;

		for (int i = 0; i < 16; i++) {
			if (keypad[i] != 0) { //key pressed
				V_[X] = i;
				keypressed = true;
				break;
			}
		}

		if (!keypressed) //no keypress
			return;

		incrementPC(2);
	}
			   break;

	//FX15 - set the delay timer to the value of reg VX
	case 0x0015: {
		delayTimer_ = V_[X];
		incrementPC(2);
	}
			   break;

	//FX18 - set the sound timer to the value of reg VX
	case 0x0018: {
		soundTimer_ = V_[X];
		incrementPC(2);
	}
			   break;

	//FX1E - add the value stored in reg VX to reg I
	case 0x001E: {
		I_ += V_[X];
		incrementPC(2);
	}
			   break;

	//FX29 - set I to the memory address of the sprite data
	//corresponding to the hex digit stored in reg VX
	//Characters 0-F (in hexadecimal) are represented by a 4x5 font
	case 0x0029: {
		byte digit = V_[X];
		I_ = 0x50 + (5 * digit);
		incrementPC(2);
	}
			   break;
	//FX33 - Store the binary-coded decimal equivalent of the value stored 
    //in register VX at addresses I, I + 1, and I + 2
	case 0x0033: {
		byte value = V_[X];
		//ones place
		ram_[I_ + 2] = (value % 10);
		value /= 10;

		//tens place
		ram_[I_ + 1] = (value % 10);
		value /= 10;

		//hundreds place
		ram_[I_] = (value % 10);
		incrementPC(2);
	}
			   break;
	
	//FX55 - store the values of registers V0 to VX inclusive in memory
	//starting at address I
	//I is set to I + X + 1 after operation
	case 0x0055: {
		for (int i = 0; i <= X; i++) {
			ram_[I_ + i] = V_[i];
		}
		I_ = I_ + X + 1;
		incrementPC(2);
	}
			   break;

	//FX65 - fill registers V0 to VX inclusive with the values
	//stored in memory starting at address I
	//I is set to I + X + 1 after operation
	case 0x0065: {
		for (int i = 0; i <= X; i++) {
			V_[i] = ram_[I_ + i];
		}
		I_ = I_ + X + 1;
		incrementPC(2);
	}
			   break;
	}
}

void Chip8::initialize()
{
	srand(time(NULL));
	PC_ = 0x200; //start of program
	I_ = 0x0000;
	SP_ = 15; //point to the top of stack, grow downward to add onto the stack

	memset(V_, 0x00, sizeof(V_));
	memset(stack_, 0x0000, sizeof(stack_));
	memset(ram_, 0x00, sizeof(ram_));
	memset(graphics, 0x00, sizeof(graphics));
	memset(keypad, 0x00, sizeof(keypad));

	delayTimer_ = 0x00;
	soundTimer_ = 0x00;

	//load font data
	//font set stored in memory from 80(0x50) - 160(0xA0)
	byte fonts[80] = {
		0xF0, 0x90, 0x90 ,0x90, 0xF0, //"0"
		0x20, 0x60, 0x20, 0x20, 0x70, //"1"
		0xF0, 0x10, 0xF0, 0x80, 0xF0, //"2"
		0xF0, 0x10, 0xF0, 0x10, 0xF0, //"3"
		0x90, 0x90, 0xF0, 0x10, 0x10, //"4"
		0xF0, 0x80, 0xF0, 0x10, 0xF0, //"5"
		0xF0, 0x80, 0xF0, 0x90, 0xF0, //"6"
		0xF0, 0x10, 0x20, 0x40, 0x40,  //"7"
		0xF0, 0x90, 0xF0, 0x90, 0xF0, //"8"
		0xF0, 0x90, 0xF0, 0x10, 0xF0, //"9"
		0xF0, 0x90, 0xF0, 0x90, 0x90, //"A"
		0xE0, 0x90, 0xE0, 0x90, 0xE0, //"B"
		0xF0, 0x80, 0x80, 0x80, 0xF0, //"C"
		0xE0, 0x90, 0x90, 0x90, 0xE0, //"D"
		0xF0, 0x80, 0xF0, 0x80, 0xF0, //"E"
		0xF0, 0x80, 0xF0, 0x80, 0x80 //"F"
	};
	for (int i = 0; i < 0x50; i++)
		ram_[0x50 + i] = fonts[i];

	initializeAudio();
}


void Chip8::initializeAudio()
{
	audiospec_ = new SDL_AudioSpec();
	audiospec_->freq = SAMPLE_RATE;
	audiospec_->format = AUDIO_S8;
	audiospec_->channels = 1;
	audiospec_->samples = 4096;
	audiospec_->callback = audio_callback;

	device_ = SDL_OpenAudioDevice(NULL, 0, audiospec_, NULL, 0);

	if (!device_) {
		std::cerr << "Failed to init audio device: " << SDL_GetError() << std::endl;
	}
}

// Samples can vary between -127 and 128
// Sine wave varies between -50 and 50
void Chip8::audio_callback(void* userdata, Uint8* raw_buffer, int bytes)
{
	for (int i = 0; i < bytes; i++) {
		raw_buffer[i] = 20 * sin(wave_position);
		wave_position += wave_increment;
	}
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

		std::cout << romFile << " loaded" << std::endl;
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


void Chip8::debugRender()
{
	system("cls");
	// Draw
	for (int y = 0; y < 32; ++y)
	{
		for (int x = 0; x < 64; ++x)
		{
			if (graphics[(y * 64) + x] == 0)
				std::cout << "*";
			else
				std::cout << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}
