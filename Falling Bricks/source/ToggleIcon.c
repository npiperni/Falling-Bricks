#include "ToggleIcon.h"
#include "ResolutionContext.h"
#include "Constants.h"
#include <stdio.h>
#include <SDL.h>

ToggleIcon* create_toggle_icon(SDL_Rect rect,  const char* texture_on_path, const char* texture_off_path) {
	SDL_Surface* surface_on = SDL_LoadBMP(texture_on_path);
	SDL_Surface* surface_off = SDL_LoadBMP(texture_off_path);
	if (!surface_on || !surface_off) {
		SDL_Log("Failed to load BMP image: %s", SDL_GetError());
		return NULL;
	}

	ToggleIcon* toggle_icon = malloc(sizeof(ToggleIcon));
	if (!toggle_icon) {
		fprintf(stderr, "Error: Failed to allocate memory for ToggleIcon\n");
		return NULL;
	}
	Uint32 colorkey = SDL_MapRGB(surface_on->format, 255, 255, 255);
	SDL_SetColorKey(surface_on, SDL_TRUE, colorkey);
	SDL_SetColorKey(surface_off, SDL_TRUE, colorkey);

	SDL_Renderer* renderer = SDL_GetRenderer(SDL_GetWindowFromID(1));
	toggle_icon->texture_on = SDL_CreateTextureFromSurface(renderer, surface_on);
	toggle_icon->texture_off = SDL_CreateTextureFromSurface(renderer, surface_off);
	SDL_FreeSurface(surface_on);
	SDL_FreeSurface(surface_off);
	if (!toggle_icon->texture_on || !toggle_icon->texture_off) {
		fprintf(stderr, "Error loading texture: %s\n", SDL_GetError());
		SDL_DestroyTexture(toggle_icon->texture_on);
		SDL_DestroyTexture(toggle_icon->texture_off);
		free(toggle_icon);
		return NULL;
	}
	toggle_icon->rect = rect;
	toggle_icon->is_toggled = true;
	toggle_icon->res_context = get_resolution_context(WINDOW_WIDTH, WINDOW_HEIGHT);
	return toggle_icon;
}

void draw_toggle_icon(ToggleIcon* toggle_icon, SDL_Renderer* renderer) {
	if (!toggle_icon) return;
	SDL_Texture* texture = toggle_icon->is_toggled ? toggle_icon->texture_on : toggle_icon->texture_off;
	SDL_Rect rect = toggle_icon->rect;
	ResolutionContext context = toggle_icon->res_context;
	rect.x = rect.x * context.scale_factor + context.x_offset;
	rect.y = rect.y * context.scale_factor + context.y_offset;
	rect.w *= context.scale_factor;
	rect.h *= context.scale_factor;
	SDL_RenderCopy(renderer, texture, NULL, &rect);
}

void destroy_toggle_icon(ToggleIcon* toggle_icon) {
	if (!toggle_icon) return;
	SDL_DestroyTexture(toggle_icon->texture_on);
	SDL_DestroyTexture(toggle_icon->texture_off);
	free(toggle_icon);
}