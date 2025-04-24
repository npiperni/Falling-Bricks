#pragma once

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "ResolutionContext.h"

typedef void (*ButtonCallback)(void);

typedef struct {
	SDL_Rect rect;
	SDL_Color color;
	bool hovered;
	ButtonCallback on_click;
	const char* label;
	SDL_Texture* texture;
	ResolutionContext res_context;
} Button;

Button* create_button(int x, int y, int width, int height, SDL_Color color, ButtonCallback on_click, const char* label, TTF_Font* font);

void destroy_button(Button* button);

void draw_button(Button* button, SDL_Renderer* renderer);

void handle_button_event(Button* button, SDL_Event event);