#pragma once

#include <SDL_ttf.h>
#include <stdbool.h>

typedef struct {
	TTF_Font* button_font;
	TTF_Font* title_font;
	TTF_Font* label_font;
	TTF_Font* label_font_small;
} FontContext;

bool create_font_context();

FontContext* get_font_context();

void adjust_label_font_size(float scale_factor);

void destroy_font_context();