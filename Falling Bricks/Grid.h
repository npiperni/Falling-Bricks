#pragma once

#include <SDL.h>
#include <stdbool.h>

#include "Piece.h"

typedef struct {
	Piece* piece;
	bool occupied;
	bool shadow;
} Cell;

typedef struct {
	Cell** cells;
	int width;
	int height;
	bool show_grid_lines;
} Grid;

Grid* create_grid(int width, int height, bool show_lines);

void destroy_grid(Grid* grid);

bool validate_grid_position(Grid* grid, int row, int col);

bool add_piece_to_grid(Grid* grid, Piece* piece, int row, int col);

void draw_grid(Grid* grid, SDL_Renderer* renderer);