#pragma once

#include <SDL.h>
#include <stdbool.h>


typedef struct {
	bool occupied;
	SDL_Color color;
	bool outline;
} Cell;

typedef struct {
	Cell** cells;
	int width;
	int height;
	bool show_grid_lines;
} Grid;

Grid* create_grid(int width, int height, bool show_lines);

void destroy_grid(Grid* grid);

void draw_grid(Grid* grid, SDL_Renderer* renderer);