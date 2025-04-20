#include "Menu.h"
#include "Button.h"
#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>

struct TitleMenu* create_title_menu(ButtonCallback on_click[4]) {
	struct TitleMenu* menu = malloc(sizeof(struct TitleMenu));
	if (!menu) {
		fprintf(stderr, "Error: Failed to allocate memory for TitleMenu\n");
		return NULL;
	}
	SDL_Color button_color = { 0, 255, 0, SDL_ALPHA_OPAQUE };
	menu->buttons[0] = create_button(100, 100, 200, 50, button_color, on_click[0]);
	menu->buttons[1] = create_button(100, 200, 200, 50, button_color, on_click[1]);
	menu->buttons[2] = create_button(100, 300, 200, 50, button_color, on_click[2]);
	menu->buttons[3] = create_button(100, 400, 200, 50, button_color, on_click[3]);
	return menu;
}

void draw_title_menu(struct TitleMenu* menu, SDL_Renderer* renderer) {
	for (int i = 0; i < 4; i++) {
		draw_button(menu->buttons[i], renderer);
	}
}

void handle_title_menu_events(struct TitleMenu* menu, SDL_Event event) {
	for (int i = 0; i < 4; i++) {
		handle_button_event(menu->buttons[i], event);
	}
}

void destroy_title_menu(struct TitleMenu* menu) {
	for (int i = 0; i < 4; i++) {
		destroy_button(menu->buttons[i]);
	}
	free(menu);
}

struct GameOverMenu* create_game_over_menu(ButtonCallback on_click[2]) {
	struct GameOverMenu* menu = malloc(sizeof(struct GameOverMenu));
	if (!menu) {
		fprintf(stderr, "Error: Failed to allocate memory for GameOverMenu\n");
		return NULL;
	}
	SDL_Color button_color = { 255, 0, 0, SDL_ALPHA_OPAQUE };
	menu->buttons[0] = create_button(100, 500, 200, 50, button_color, on_click[0]);
	menu->buttons[1] = create_button(100, 600, 200, 50, button_color, on_click[1]);
	return menu;
}

void draw_game_over_menu(struct GameOverMenu* menu, SDL_Renderer* renderer) {
	for (int i = 0; i < 2; i++) {
		draw_button(menu->buttons[i], renderer);
	}
}

void handle_game_over_menu_events(struct GameOverMenu* menu, SDL_Event event) {
	for (int i = 0; i < 2; i++) {
		handle_button_event(menu->buttons[i], event);
	}
}

void destroy_game_over_menu(struct GameOverMenu* menu) {
	for (int i = 0; i < 2; i++) {
		destroy_button(menu->buttons[i]);
	}
	free(menu);
}