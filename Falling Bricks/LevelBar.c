#include "LevelBar.h"
#include <SDL.h>

void draw_level_bar(SDL_Renderer* renderer, int x, int y, int w, int h, int lines, int goal) {
	int border_width = 4;
	SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
	SDL_Rect top_border = { x, y, w + border_width * 2, border_width };
	SDL_Rect bottom_border = { x, y + h + border_width, w + border_width * 2, border_width };
	SDL_Rect left_border = { x, y + border_width, border_width, h };
	SDL_Rect right_border = { x + w + border_width, y + border_width, border_width, h };
	SDL_RenderFillRect(renderer, &top_border);
	SDL_RenderFillRect(renderer, &bottom_border);
	SDL_RenderFillRect(renderer, &left_border);
	SDL_RenderFillRect(renderer, &right_border);

	SDL_SetRenderDrawColor(renderer, 233, 200, 0, 255);
	int level_height = (int)((float)h * lines / goal);
	SDL_Rect rect = { x + border_width, y + border_width + h - level_height, w, level_height };
	SDL_RenderFillRect(renderer, &rect);
}