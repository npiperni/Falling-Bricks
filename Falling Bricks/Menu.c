#include "Menu.h"
#include "Button.h"
#include "Constants.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>

struct TitleMenu* create_title_menu(ButtonCallback on_click[4], TTF_Font* title_font, TTF_Font* button_font) {
	struct TitleMenu* menu = malloc(sizeof(struct TitleMenu));
	if (!menu) {
		fprintf(stderr, "Error: Failed to allocate memory for TitleMenu\n");
		return NULL;
	}
	SDL_Color button_color = { 50, 50, 50, SDL_ALPHA_OPAQUE };
	int button_width = 200;
	int button_height = 50;
	int button_x = (WINDOW_WIDTH - button_width) / 2;
	int button_y = WINDOW_HEIGHT / 2 - 200;
	menu->buttons[0] = create_button(button_x, button_y + 100, button_width, button_height, button_color, on_click[0], "40 Lines", button_font);
	menu->buttons[1] = create_button(button_x, button_y + 200, button_width, button_height, button_color, on_click[1], "Blitz", button_font);
	menu->buttons[2] = create_button(button_x, button_y + 300, button_width, button_height, button_color, on_click[2], "Endless", button_font);
	menu->buttons[3] = create_button(button_x, button_y + 400, button_width, button_height, button_color, on_click[3], "Quit", button_font);
	menu->scale_factor = 1.0f;

	SDL_Surface* title_surface = TTF_RenderText_Solid(title_font, "Falling Bricks", (SDL_Color) { 200, 175, 0, SDL_ALPHA_OPAQUE });
	menu->title_texture = SDL_CreateTextureFromSurface(SDL_GetRenderer(SDL_GetWindowFromID(1)), title_surface);
	SDL_FreeSurface(title_surface);
	return menu;
}

void draw_title_menu(struct TitleMenu* menu, SDL_Renderer* renderer) {
	SDL_Rect title_rect = { (WINDOW_WIDTH - 600) / 2, 50, 600, 100 };
	title_rect.x *= menu->scale_factor;
	title_rect.y *= menu->scale_factor;
	title_rect.w *= menu->scale_factor;
	title_rect.h *= menu->scale_factor;
	SDL_RenderCopy(renderer, menu->title_texture, NULL, &title_rect);

	for (int i = 0; i < 4; i++) {
		menu->buttons[i]->scale_factor = menu->scale_factor;
		draw_button(menu->buttons[i], renderer);
	}
}

void handle_title_menu_events(struct TitleMenu* menu, SDL_Event event) {
	for (int i = 0; i < 4; i++) {
		handle_button_event(menu->buttons[i], event);
	}
}

void destroy_title_menu(struct TitleMenu* menu) {
	if (!menu) return;
	SDL_DestroyTexture(menu->title_texture);
	for (int i = 0; i < 4; i++) {
		destroy_button(menu->buttons[i]);
	}
	free(menu);
}

struct GameOverMenu* create_game_over_menu(ButtonCallback on_click[2], TTF_Font* button_font) {
	struct GameOverMenu* menu = malloc(sizeof(struct GameOverMenu));
	if (!menu) {
		fprintf(stderr, "Error: Failed to allocate memory for GameOverMenu\n");
		return NULL;
	}
	SDL_Color button_color = { 50, 50, 50, SDL_ALPHA_OPAQUE };
	int button_width = 200;
	int button_height = 50;
	int button_x = 100;
	int button_y = (WINDOW_HEIGHT - button_height) / 2;
	menu->buttons[0] = create_button(button_x, button_y + 100, button_width, button_height, button_color, on_click[0], "Menu", button_font);
	menu->buttons[1] = create_button(button_x, button_y + 200, button_width, button_height, button_color, on_click[1], "Quit", button_font);
	menu->scale_factor = 1.0f;
	return menu;
}

void draw_game_over_menu(struct GameOverMenu* menu, SDL_Renderer* renderer) {
	for (int i = 0; i < 2; i++) {
		menu->buttons[i]->scale_factor = menu->scale_factor;
		draw_button(menu->buttons[i], renderer);
	}
}

void handle_game_over_menu_events(struct GameOverMenu* menu, SDL_Event event) {
	for (int i = 0; i < 2; i++) {
		handle_button_event(menu->buttons[i], event);
	}
}

void destroy_game_over_menu(struct GameOverMenu* menu) {
	if (!menu) return;
	for (int i = 0; i < 2; i++) {
		destroy_button(menu->buttons[i]);
	}
	free(menu);
}