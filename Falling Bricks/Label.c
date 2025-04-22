#include "Label.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

SDL_Rect draw_label(SDL_Renderer* renderer, int x, int y, const char* label, TTF_Font* font, bool alignRight, bool alignBottom) {
	SDL_Surface* surface = TTF_RenderText_Blended(font, label, (SDL_Color) { 200, 200, 200, SDL_ALPHA_OPAQUE });
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect rect = { x, y, surface->w, surface->h };
	if (alignRight) {
		rect.x -= surface->w;
	}
	if (alignBottom) {
		rect.y -= surface->h;
	}
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	return rect;
}

void time_formater(char* mins_secs_buffer, char* millis_buffer, size_t size_of_buffers, int time_in_ms) {
	int minutes = time_in_ms / 60000;
	int seconds = (time_in_ms % 60000) / 1000;
	int milliseconds = time_in_ms % 1000;
	snprintf(mins_secs_buffer, size_of_buffers, "%d:%02d", minutes, seconds);
	snprintf(millis_buffer, size_of_buffers, ".%03d", milliseconds);
}