#include "Label.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

SDL_Rect draw_label(SDL_Renderer* renderer, int x, int y, const char* label, TTF_Font* font, bool alignRight) {
	SDL_Surface* surface = TTF_RenderText_Blended(font, label, (SDL_Color) { 200, 200, 200, SDL_ALPHA_OPAQUE });
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect rect = { x, y, surface->w, surface->h };
	if (alignRight) {
		rect.x -= surface->w;
	}
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	return rect;
}
