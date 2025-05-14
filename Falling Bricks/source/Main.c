#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <time.h>

#include "Constants.h"
#include "Paths.h"
#include "Game.h"

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
#endif

// For browser support
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
bool game_is_running = false;

bool init_window(SDL_Window** window, SDL_Renderer** renderer) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "Error initializing SDL.\n");
		return false;
	}
	SDL_Window* new_window = SDL_CreateWindow(
		"Falling Bricks",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		//SDL_WINDOW_BORDERLESS
		SDL_WINDOW_RESIZABLE
	);
	if (!new_window) {
		fprintf(stderr, "Error creating SDL window.\n");
		return false;
	}
	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_Renderer* new_renderer = SDL_CreateRenderer(new_window, -1, SDL_RENDERER_ACCELERATED);
	if (!new_renderer) {
		fprintf(stderr, "Error creating SDL renderer.\n");
		return false;
	}
	SDL_SetRenderDrawBlendMode(new_renderer, SDL_BLENDMODE_BLEND);

	// Set window icon
	SDL_Surface* icon_surface = SDL_LoadBMP(ICON_PATH);
	if (icon_surface) {
		Uint32 colorkey = SDL_MapRGB(icon_surface->format, 255, 255, 255);
		SDL_SetColorKey(icon_surface, SDL_TRUE, colorkey);
		SDL_SetWindowIcon(new_window, icon_surface);
		SDL_FreeSurface(icon_surface);
	}
	else {
		fprintf(stderr, "Error loading window icon: %s\n", SDL_GetError());
	}

	*window = new_window;
	*renderer = new_renderer;
	return true;
}

void destroy_window(SDL_Window* window, SDL_Renderer* renderer) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

// Main game loop function for WebAssembly
#ifdef __EMSCRIPTEN__
void main_loop() {
	if (!game_is_running) {
		emscripten_cancel_main_loop();
	}

	process_input(&game_is_running);
	update();
	render(renderer);
}
#endif

int main(int argc, char* args[]) {
	srand(time(NULL));
#ifdef _DEBUG
	// Enable memory leak checks only in debug builds
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	game_is_running = init_window(&window, &renderer) && setup();

#ifdef __EMSCRIPTEN__
	// If running in a browser, use emscripten's game loop
	emscripten_set_main_loop(main_loop, 0, 1);
#else
	// If running natively, use the traditional game loop
	while (game_is_running) {
		process_input(&game_is_running);
		update();
		render(renderer);
	}
#endif

	cleanup();

	destroy_window(window, renderer);

	return EXIT_SUCCESS;
}