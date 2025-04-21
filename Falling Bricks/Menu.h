#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "Button.h"

struct TitleMenu {
	Button* buttons[4];
	SDL_Texture* title_texture;
	float scale_factor;
};

struct GameOverMenu {
	Button* buttons[2];
	float scale_factor;
};

struct TitleMenu* create_title_menu(ButtonCallback on_click[4], TTF_Font* title_font, TTF_Font* button_font);

void draw_title_menu(struct TitleMenu* menu, SDL_Renderer* renderer);

void handle_title_menu_events(struct TitleMenu* menu, SDL_Event event);

void destroy_title_menu(struct TitleMenu* menu);

struct GameOverMenu* create_game_over_menu(ButtonCallback on_click[2], TTF_Font* button_font);

void draw_game_over_menu(struct GameOverMenu* menu, SDL_Renderer* renderer);

void handle_game_over_menu_events(struct GameOverMenu* menu, SDL_Event event);

void destroy_game_over_menu(struct GameOverMenu* menu);