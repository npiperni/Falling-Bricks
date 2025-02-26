#pragma once

#include <SDL.h>
#include <stdbool.h>

#include "Piece.h"

typedef struct {
	Piece* piece;
	bool locked;
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

bool validate_piece_position(Grid* grid, Piece* piece, int row, int col);

bool add_piece_to_grid(Grid* grid, Piece* piece, int row, int col, bool lock);

void clear_unlocked_cells(Grid* grid);

void draw_grid(Grid* grid, SDL_Renderer* renderer);