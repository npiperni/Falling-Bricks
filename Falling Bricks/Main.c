#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <time.h>

#include "Constants.h"
#include "Game.h"

TTF_Font* font = NULL;

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
	*window = new_window;
	*renderer = new_renderer;
	return true;
}

bool load_fonts() {
	if (TTF_Init() == -1) {
		fprintf(stderr, "Error initializing TTF: %s\n", TTF_GetError());
		return false;
	}
	font = TTF_OpenFont("StarlightRegular-7m9E.ttf", 24);
	if (!font) {
		fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
		return false;
	}
}

void free_fonts() {
	if (font) {
		TTF_CloseFont(font);
		font = NULL;
	}
	TTF_Quit();
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

	bool game_is_running = init_window(&window, &renderer) && load_fonts() && setup();

	while (game_is_running) {
		process_input(&game_is_running);
		update();
		render(renderer);
	}

	cleanup();

	destroy_window(window, renderer);

	free_fonts();

	return 0;
}