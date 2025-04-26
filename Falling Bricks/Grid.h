#pragma once

#include <SDL.h>
#include <stdbool.h>
#include "Piece.h"
#include "DynamicArray.h"

typedef struct {
	Piece* piece;
	bool locked;
	bool shadow;
	bool x;
} Cell;

typedef struct {
	Cell** cells;
	int width;
	int height;
	bool show_grid_lines;
	bool is_game_board;
	bool* full_rows;
	Uint32 fade_start_time;
	DynamicArray* locked_pieces;
} Grid;

Grid* create_grid(int width, int height, bool show_lines, bool is_game_board);

void destroy_grid(Grid* grid);

bool validate_piece_position(Grid* grid, Piece* piece);

bool validate_piece_at_position(Grid* grid, Piece* piece, int row, int col);

bool add_piece_to_grid(Grid* grid, Piece* piece, bool lock, bool drop);

void mark_x_cells(Grid* grid, Piece* piece);

Piece* try_rotate_piece(Grid* grid, Piece* piece);

void clear_unlocked_cells(Grid* grid);

void clear_grid(Grid* grid);

void draw_grid(Grid* grid, int origin_x, int origin_y, int cell_width, int border_width, SDL_Renderer* renderer);

int check_and_mark_full_rows(Grid* grid);

void clear_full_rows(Grid* grid);

void drop_all_pieces(Grid* grid);