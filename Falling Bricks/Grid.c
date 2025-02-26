#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "Grid.h"

static bool allocate_cells(Grid* grid);
static void deallocate_cells(Cell** cells, int height);
static void init_cells_in_row(Cell* cells, int width);

Grid* create_grid(int width, int height, bool show_lines) {
	Grid* grid = malloc(sizeof(Grid));
	if (!grid) {
		fprintf(stderr, "Error: Failed to allocate memory for Grid\n");
		return NULL;
	}
	grid->width = width;
	grid->height = height;
	grid->show_grid_lines = show_lines;

	if (!allocate_cells(grid)) {
		return NULL;
	}
	
	return grid;
}

void destroy_grid(Grid* grid) {
	deallocate_cells(grid->cells, grid->height);
	free(grid);
}

bool validate_grid_position(Grid* grid, int row, int col) {
	return row >= 0 && row < grid->height && col >= 0 && col < grid->width && !grid->cells[row][col].occupied;
}

bool add_piece_to_grid(Grid* grid, Piece* piece, int row, int col) {
	for (int i = 0; i < piece->height; i++) {
		for (int j = 0; j < piece->width; j++) {
			if (piece->shape[i * piece->width + j]) {
				if (!validate_grid_position(grid, row + i, col + j)) {
					return false;
				}
			}
		}
	}
	for (int i = 0; i < piece->height; i++) {
		for (int j = 0; j < piece->width; j++) {
			if (piece->shape[i * piece->width + j]) {
				grid->cells[row + i][col + j].piece = piece;
				//grid->cells[row + i][col + j].occupied = true;
			}
		}
	}
}

void draw_grid(Grid* grid, SDL_Renderer* renderer) {
	int origin_x = 100;
	int origin_y = 100;
	int cell_width = 32;
	for (int i = 0; i < grid->height; i++) {
		for (int j = 0; j < grid->width; j++) {
			SDL_Rect cell_rect = {
				j * cell_width + origin_x,
				i * cell_width + origin_y,
				cell_width,
				cell_width
			};
			if (grid->show_grid_lines) {
				SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
				SDL_RenderDrawRect(renderer, &cell_rect);
			}
			Piece* piece = grid->cells[i][j].piece;
			if (piece) {
				SDL_SetRenderDrawColor(renderer, piece->color.r, piece->color.g, piece->color.b, piece->color.a);
				SDL_RenderFillRect(renderer, &cell_rect);

				// Draw shadow around block to make it look 3D from far
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128);
				SDL_Rect top_outline = { cell_rect.x, cell_rect.y, cell_width, cell_width / 8 };
				SDL_Rect right_outline = { cell_rect.x + cell_width - cell_width / 8, cell_rect.y + cell_width / 8, cell_width / 8, cell_width - cell_width / 4 };
				SDL_RenderFillRect(renderer, &top_outline);
				SDL_RenderFillRect(renderer, &right_outline);

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
				SDL_Rect bottom_outline = { cell_rect.x, cell_rect.y + cell_width - cell_width / 8, cell_width, cell_width / 8 };
				SDL_Rect left_outline = { cell_rect.x, cell_rect.y + cell_width / 8, cell_width / 8, cell_width - cell_width / 4 };
				SDL_RenderFillRect(renderer, &bottom_outline);
				SDL_RenderFillRect(renderer, &left_outline);
			}
			
			//if (grid->cells[i][j].shadow) {
			//	
			//}
		}
	}
}

static bool allocate_cells(Grid* grid) {
	grid->cells = malloc(sizeof(Cell*) * grid->height);
	if (!grid->cells) {
		fprintf(stderr, "Error: Failed to allocate memory for Grid cells\n");
		free(grid);
		return false;
	}
	for (int i = 0; i < grid->height; i++) {
		grid->cells[i] = malloc(sizeof(Cell) * grid->width);
		if (!grid->cells[i]) {
			fprintf(stderr, "Error: Failed to allocate memory for Grid cells\n");
			deallocate_cells(grid->cells, i);
			free(grid);
			return false;
		}
		init_cells_in_row(grid->cells[i], grid->width);
	}
}

static void deallocate_cells(Cell** cells, int height) {
	for (int i = 0; i < height; i++) {
		free(cells[i]);
	}
	free(cells);
}

static void init_cells_in_row(Cell* cells, int width) {
	for (int i = 0; i < width; i++) {
		cells[i].piece = NULL;
		cells[i].occupied = false;
		cells[i].shadow = false;
	}
}