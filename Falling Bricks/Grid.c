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

void draw_grid(Grid* grid, SDL_Renderer* renderer) {
	int cell_width = 36;
	for (int i = 0; i < grid->height; i++) {
		for (int j = 0; j < grid->width; j++) {
			SDL_Rect cell_rect = {
				j * cell_width,
				i * cell_width,
				cell_width,
				cell_width
			};
			if (grid->show_grid_lines) {
				SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
				SDL_RenderDrawRect(renderer, &cell_rect);
			}
			SDL_SetRenderDrawColor(renderer, grid->cells[i][j].color.r, grid->cells[i][j].color.g, grid->cells[i][j].color.b, grid->cells[i][j].color.a);
			SDL_RenderFillRect(renderer, &cell_rect);
			if (grid->cells[i][j].outline) {
				SDL_SetRenderDrawColor(renderer, 0, 177, 0, 255);
				SDL_Rect top_outline = { cell_rect.x, cell_rect.y, cell_rect.w, cell_width / 4 };
				SDL_Rect right_outline = { cell_rect.x + cell_width - cell_width / 4, cell_rect.y, cell_width / 4, cell_rect.h };
				SDL_Rect bottom_outline = { cell_rect.x, cell_rect.y + cell_width - cell_width / 4, cell_rect.w, cell_width / 4 };
				SDL_Rect left_outline = { cell_rect.x, cell_rect.y, cell_width / 4, cell_rect.h };
				SDL_Rect middle = { cell_rect.x + cell_width / 4, cell_rect.y + cell_width / 4, cell_width / 2, cell_width / 2 };
				/*SDL_RenderFillRect(renderer, &top_outline);
				SDL_RenderFillRect(renderer, &right_outline);
				SDL_RenderFillRect(renderer, &bottom_outline);
				SDL_RenderFillRect(renderer, &left_outline);*/
				SDL_RenderFillRect(renderer, &middle);
			}
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
		cells[i].color.r = 0;
		cells[i].color.g = 0;
		cells[i].color.b = 0;
		cells[i].color.a = 0;
		cells[i].outline = false;
		cells[i].occupied = false;
	}
}