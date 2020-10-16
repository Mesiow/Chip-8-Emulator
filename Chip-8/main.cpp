#include <iostream>
#include <chrono>
#include "Chip8.h"


int main(int argc, char *argv[]) {

	const int width = 64;
	const int height = 32;
	int scale = 8;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		printf("SDL FAILED TO INIT");

	Chip8 c8;
	c8.loadRom("Roms/Breakout.ch8");

	SDL_Window* window = SDL_CreateWindow("Chip-8", 128, 128, width * scale, height * scale, SDL_WINDOW_SHOWN);
	if (!window) {
		std::cerr << "SDL Window failed to create" << std::endl;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << "SDL Renderer failed to create" << std::endl;
		return -3;
	}
	SDL_Texture* texture = nullptr;
	SDL_Surface* surface = nullptr;

	
	SDL_Event e;
	bool running = true;

	float cycleDelay = 5.0f;
	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	while (running) {
		while (SDL_PollEvent(&e)) {
			c8.checkInput(e);
			switch (e.type) {
			case SDL_QUIT: running = false;
			}
		}


		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		if (dt > cycleDelay) {
			lastCycleTime = currentTime;
			c8.run();

			//Use this function to allocate a new RGB surface with existing pixel data.
			surface = SDL_CreateRGBSurfaceFrom(c8.graphics, width, height,
				32, width * 4, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

			texture = SDL_CreateTextureFromSurface(renderer, surface);

			SDL_RenderClear(renderer); //clear screen
			SDL_RenderCopy(renderer, texture, nullptr, nullptr);
			SDL_RenderPresent(renderer); //update screen
		}
	}
	



	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}