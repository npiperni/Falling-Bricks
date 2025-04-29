#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <time.h>

#include "Constants.h"
#include "Game.h"

TTF_Font* button_font = NULL;
TTF_Font* title_font = NULL;
TTF_Font* label_font = NULL;
TTF_Font* label_font_small = NULL;

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
	button_font = TTF_OpenFont("AmericanCaptain-MdEY.otf", 72);
	if (!button_font) {
		fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
		return false;
	}
	title_font = TTF_OpenFont("BubblePixel7-do9x.ttf", 128);
	if (!title_font) {
		fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
		return false;
	}
	label_font = TTF_OpenFont("PoltBold-V5aZ.otf", LABEL_DEFAULT_FONT_SIZE);
	if (!label_font) {
		fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
		return false;
	}
	label_font_small = TTF_OpenFont("Polt-AABM.otf", LABEL_DEFAULT_SMALL_FONT_SIZE);
	if (!label_font_small) {
		fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
		return false;
	}
}

void free_fonts() {
	TTF_CloseFont(button_font);
	button_font = NULL;
	TTF_CloseFont(title_font);
	title_font = NULL;
	TTF_CloseFont(label_font);
	label_font = NULL;
	TTF_CloseFont(label_font_small);
	label_font_small = NULL;
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

	if (!load_fonts()) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "MISSING FONTS", "Failed to load required fonts.", 0);
		free_fonts();
		return EXIT_FAILURE;
	}

	bool game_is_running = init_window(&window, &renderer) && setup();

	while (game_is_running) {
		process_input(&game_is_running);
		update();
		render(renderer);
	}

	cleanup();

	destroy_window(window, renderer);

	free_fonts();

	return EXIT_SUCCESS;
}