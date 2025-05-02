#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "Button.h"
#include "ResolutionContext.h"
#include "DynamicArray.h"

#define BLOCK_INTERVAL 1500

struct TitleMenu {
	Button* buttons[4];
	SDL_Texture* title_texture;
	ResolutionContext res_context;
	DynamicArray* floating_grids;
	DynamicArray* grid_positions;
	Uint32 floating_grid_creation_time;
};

struct GameOverMenu {
	Button* buttons[2];
	ResolutionContext res_context;
};

struct TitleMenu* create_title_menu(ButtonCallback on_click[4]);

void draw_title_menu(struct TitleMenu* menu, SDL_Renderer* renderer);

void update_grid_positions(struct TitleMenu* menu, float delta_time);

void handle_title_menu_events(struct TitleMenu* menu, SDL_Event event);

void destroy_title_menu(struct TitleMenu* menu);

struct GameOverMenu* create_game_over_menu(ButtonCallback on_click[2]);

void draw_game_over_menu(struct GameOverMenu* menu, SDL_Renderer* renderer);

void handle_game_over_menu_events(struct GameOverMenu* menu, SDL_Event event);

void destroy_game_over_menu(struct GameOverMenu* menu);