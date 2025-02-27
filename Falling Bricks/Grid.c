#include "Grid.h"
#include <stdio.h>

static bool allocate_cells(Grid* grid);
static void deallocate_cells(Cell** cells, int height);
static void init_cells_in_row(Cell* cells, int width);
static bool validate_grid_position(Grid* grid, int row, int col);
static bool insert_piece_at_position(Grid* grid, Piece* piece, int row, int col, bool lock);
static bool drop_piece_on_grid(Grid* grid, Piece* piece, int row, int col, bool lock);
static void mark_shadow_predictions(Grid* grid, Piece* piece, int row, int col);

// Possible position shifts (row, col) to check after rotation
static const int wall_kick_attempts[10][2] = {
	{  0,  0 }, // Default position
	{  0, -1 }, // Left 1
	{  0, +1 }, // Right 1
	{ +1,  0 }, // Down 1
	{ -1,  0 }, // Up 1
	{ +1, -1 }, // Down-left
	{ +1, +1 }, // Down-right
	{ -1, +1 }, // Up-right
	{  0, +2 }, // Long piece needs more room
	{ +2,  0 }  // Long piece needs more room
};

Grid* create_grid(int width, int height, bool show_lines) {
	Grid* grid = malloc(sizeof(Grid));
	if (!grid) {
		fprintf(stderr, "Error: Failed to allocate memory for Grid\n");
		return NULL;
	}
	grid->locked_pieces = create_dynamic_array(10, destroy_piece);
	if (!grid->locked_pieces) {
		free(grid);
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
	destroy_dynamic_array(grid->locked_pieces);
	free(grid);
}

bool validate_piece_position(Grid* grid, Piece* piece, int row, int col) {
	for (int i = 0; i < piece->height; i++) {
		for (int j = 0; j < piece->width; j++) {
			if (piece->shape[i * piece->width + j]) {
				if (!validate_grid_position(grid, row + i, col + j)) {
					return false;
				}
			}
		}
	}
	return true;
}


bool add_piece_to_grid(Grid* grid, Piece* piece, int row, int col, bool lock, bool drop) {
	if (drop) {
		return drop_piece_on_grid(grid, piece, row, col, lock);
	}

	if (!insert_piece_at_position(grid, piece, row, col, lock)) {
		return false;
	}

	// Predict where the piece will fall and mark those cells as shadow
	if (!lock) {
		mark_shadow_predictions(grid, piece, row, col);
	}

	return true;
}

static void mark_shadow_predictions(Grid* grid, Piece* piece, int row, int col) {
	for (int i = 1; i <= grid->height - row; i++) {
		if (!validate_piece_position(grid, piece, row + i, col)) {
			for (int j = 0; j < piece->height; j++) {
				for (int k = 0; k < piece->width; k++) {
					bool shape_cell = piece->shape[j * piece->width + k];
					Piece* grid_piece = grid->cells[row + i - 1 + j][col + k].piece;
					// Draw a shadow where the piece will fall. Don't draw shadow on the piece itself if partially covered.
					if (shape_cell && grid_piece != piece) {
						grid->cells[row + i - 1 + j][col + k].shadow = true;
					}
				}
			}
			break;
		}
	}
}

static bool drop_piece_on_grid(Grid* grid, Piece* piece, int row, int col, bool lock) {
	// Predict where the piece will fall and mark those cells as shadow
	for (int i = 1; i <= grid->height - row; i++) {
		if (!validate_piece_position(grid, piece, row + i, col)) {
			return insert_piece_at_position(grid, piece, row + i - 1, col, lock);
		}
	}
	return true;
}

// Also returns new row and column position for center rotation
Piece* try_rotate_piece(Grid* grid, Piece* piece, int* row, int* col) {
	Piece* rotated_piece = rotate_piece(piece);
	if (!rotated_piece) {
		return NULL;
	}

	int center_row = *row + piece->height / 2;
	int center_col = *col + piece->width / 2;

	int new_row = center_row - rotated_piece->height / 2;
	int new_col = center_col - rotated_piece->width / 2;

	// Try all possible wall kick positions
	for (int i = 0; i < 10; i++) {
		int attempt_row = new_row + wall_kick_attempts[i][0];
		int attempt_col = new_col + wall_kick_attempts[i][1];

		if (validate_piece_position(grid, rotated_piece, attempt_row, attempt_col)) {
			*row = attempt_row;
			*col = attempt_col;
			return rotated_piece;
		}
	}

	// If all attempts fail, discard rotation
	destroy_piece(rotated_piece);
	return NULL;
}

void clear_unlocked_cells(Grid* grid) {
	for (int i = 0; i < grid->height; i++)
	{
		for (int j = 0; j < grid->width; j++)
		{
			if (!grid->cells[i][j].locked)
			{
				grid->cells[i][j].piece = NULL;
				grid->cells[i][j].shadow = false;
			}
		}
	}
}

void clear_all_cells(Grid* grid) {
	for (int i = 0; i < grid->height; i++)
	{
		for (int j = 0; j < grid->width; j++)
		{
			grid->cells[i][j].piece = NULL;
			grid->cells[i][j].shadow = false;
			grid->cells[i][j].locked = false;
		}
	}
}

void draw_grid(Grid* grid, int origin_x, int origin_y, int cell_width, bool border, SDL_Renderer* renderer) {
	int border_width = 4;

	if (border) {
		SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
		// Draw a border around the grid, use the outline code below
		SDL_Rect top_border = { origin_x, origin_y, grid->width * cell_width + border_width * 2, border_width };
		SDL_Rect bottom_border = { origin_x, origin_y + grid->height * cell_width + border_width, grid->width * cell_width + border_width * 2, border_width };
		SDL_Rect left_border = { origin_x, origin_y + border_width, border_width, grid->height * cell_width };
		SDL_Rect right_border = { origin_x + grid->width * cell_width + border_width, origin_y + border_width, border_width, grid->height * cell_width };
		SDL_RenderFillRect(renderer, &top_border);
		SDL_RenderFillRect(renderer, &bottom_border);
		SDL_RenderFillRect(renderer, &left_border);
		SDL_RenderFillRect(renderer, &right_border);
	}

	for (int i = 0; i < grid->height; i++) {
		for (int j = 0; j < grid->width; j++) {
			SDL_Rect cell_rect = {
				j * cell_width + origin_x + border_width,
				i * cell_width + origin_y + border_width,
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

				// Draw shadow / outline around block to make it look 3D from far
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
			
			if (grid->cells[i][j].shadow) {
				SDL_SetRenderDrawColor(renderer, 128, 128, 128, 128);
				SDL_RenderFillRect(renderer, &cell_rect);
			}
		}
	}
}

static bool insert_piece_at_position(Grid* grid, Piece* piece, int row, int col, bool lock) {
	if (!validate_piece_position(grid, piece, row, col)) {
		return false;
	}

	// Draw every cell of the piece to it's corresponding cell in the grid
	for (int i = 0; i < piece->height; i++) {
		for (int j = 0; j < piece->width; j++) {
			if (piece->shape[i * piece->width + j]) {
				grid->cells[row + i][col + j].piece = piece;
				if (lock){
					grid->cells[row + i][col + j].locked = true;
				}
			}
		}
	}
}

static bool validate_grid_position(Grid* grid, int row, int col) {
	return row >= 0 && row < grid->height && col >= 0 && col < grid->width && !grid->cells[row][col].locked;
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
		cells[i].locked = false;
		cells[i].shadow = false;
	}
}