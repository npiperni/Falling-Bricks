#pragma once
#include <stdbool.h>
#include <SDL.h>
#include "ResolutionContext.h"

typedef struct {
	SDL_Texture* texture_on;
	SDL_Texture* texture_off;
	SDL_Rect rect;
	bool is_toggled;
	ResolutionContext res_context;
} ToggleIcon;

ToggleIcon* create_toggle_icon(SDL_Rect rect, const char* texture_on_path, const char* texture_off_path);

void destroy_toggle_icon(ToggleIcon* toggle_icon);

void draw_toggle_icon(ToggleIcon* toggle_icon, SDL_Renderer* renderer);