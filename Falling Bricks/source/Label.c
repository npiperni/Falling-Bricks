#include "Label.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

SDL_Rect draw_label(SDL_Renderer* renderer, int x, int y, const char* label, LabelStyle label_style) {
	if (label_style.font == NULL || label == NULL || label[0] == '\0') {
		return (SDL_Rect) { 0, 0, 0, 0 };
	}
	SDL_Surface* surface = TTF_RenderText_Blended(label_style.font, label, label_style.color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(texture, label_style.color.a);

	SDL_Rect rect = { x, y, surface->w, surface->h };
	if (label_style.align_right) {
		rect.x -= surface->w;
	}
	if (label_style.align_bottom) {
		rect.y -= surface->h;
	}
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	return rect;
}

LabelStyle default_label_style_no_font() {
	return (LabelStyle) {
		.color = { 200, 200, 200, SDL_ALPHA_OPAQUE },
			.align_right = true,
			.align_bottom = true,
			.font = NULL
	};
}

void time_formater(char* mins_secs_buffer, char* millis_buffer, size_t size_of_buffers, Uint32 time_in_ms) {
	int minutes = time_in_ms / 60000;
	int seconds = (time_in_ms % 60000) / 1000;
	int milliseconds = time_in_ms % 1000;
	snprintf(mins_secs_buffer, size_of_buffers, "%d:%02d", minutes, seconds);
	snprintf(millis_buffer, size_of_buffers, ".%03d", milliseconds);
}

const char* get_row_clear_label(int rows) {
	switch (rows) {
	case 1: return "SINGLE";
	case 2: return "DOUBLE";
	case 3: return "TRIPLE";
	case 4: return "QUADRUPLE";
	default: return "";
	}
}

