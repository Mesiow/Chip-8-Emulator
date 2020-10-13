#include <iostream>
#include "Chip8.h"

int main(int argc, char *argv[]) {
	Chip8 c8;
	c8.loadRom("Roms/pong.ch8");
	return 0;
}