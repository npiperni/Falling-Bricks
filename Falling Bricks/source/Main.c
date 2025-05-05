#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <time.h>

#include "Constants.h"
#include "Paths.h"
#include "Game.h"

bool init_window(SDL_Window** window, SDL_Renderer** renderer) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error intitializing SDL.\n");
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

int main(int argc, char* args[]) {
	srand(time(NULL));

	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	bool game_is_running = init_window(&window, &renderer) && setup();

	while (game_is_running) {
		process_input(&game_is_running);
		update();
		render(renderer);
	}

	cleanup();

	destroy_window(window, renderer);

	return EXIT_SUCCESS;
}