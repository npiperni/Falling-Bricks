#include "Grid.h"
#include <stdio.h>
#include "Constants.h"

static bool allocate_cells(Grid* grid);
static void deallocate_cells(Cell** cells, int height);
static void init_cells_in_row(Cell* cells, int width);
static bool validate_grid_position(Grid* grid, int row, int col);
static bool insert_piece(Grid* grid, Piece* piece, bool lock);
static bool drop_piece_on_grid(Grid* grid, Piece* piece, bool lock);
static void mark_shadow_predictions(Grid* grid, Piece* piece);
static void remove_empty_pieces(Grid* grid);
static void clear_x_cells(Grid* grid);

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

static bool is_near_height_limit(Grid* grid) {
	SDL_assert(grid->height > 4);
	// Check if there is a locked piece in the top 4 rows
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < grid->width; col++) {
			if (grid->cells[row][col].locked && grid->cells[row][col].piece) {
				return true;
			}
		}
	}
	return false;
}

Grid* create_grid(int width, int height, bool show_lines, bool is_game_board) {
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
	grid->is_game_board = is_game_board;

	if (!allocate_cells(grid)) {
		return NULL;
	}
	
	return grid;
}

void destroy_grid(Grid* grid) {
	if (grid) {
		deallocate_cells(grid->cells, grid->height);
		destroy_dynamic_array(grid->locked_pieces);
		free(grid);
	}
}

bool validate_piece_at_position(Grid* grid, Piece* piece, int row, int col) {
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

bool validate_piece_position(Grid* grid, Piece* piece) {
	return validate_piece_at_position(grid, piece, piece->row_pos, piece->col_pos);
}


bool add_piece_to_grid(Grid* grid, Piece* piece, bool lock, bool drop) {
	if (drop) {
		return drop_piece_on_grid(grid, piece, lock);
	}

	if (!insert_piece(grid, piece, lock)) {
		return false;
	}

	// Predict where the piece will fall and mark those cells as shadow
	if (!lock) {
		mark_shadow_predictions(grid, piece);
	}

	return true;
}

static void mark_shadow_predictions(Grid* grid, Piece* piece) {
	int row = piece->row_pos;
	int col = piece->col_pos;
	for (int i = 1; i <= grid->height - row; i++) {
		if (!validate_piece_at_position(grid, piece, row + i, col)) {
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

static bool drop_piece_on_grid(Grid* grid, Piece* piece, bool lock) {
	int row = piece->row_pos;
	int col = piece->col_pos;
	if (!validate_piece_at_position(grid, piece, row, col)) {
		return false; // Don't even bother attempting to drop if its current position is invalid. Can cause problems.
	}
	for (int i = 0; i <= grid->height - row; i++) {
		if (!validate_piece_at_position(grid, piece, row + i, col)) {
			piece->row_pos = row + i - 1;
			return insert_piece(grid, piece, lock);
		}
	}
	return false;
}

// Also returns new row and column position for center rotation
Piece* try_rotate_piece(Grid* grid, Piece* piece) {
	Piece* rotated_piece = rotate_piece(piece);
	if (!rotated_piece) {
		return NULL;
	}

	int center_row = piece->row_pos + piece->height / 2;
	int center_col = piece->col_pos + piece->width / 2;

	int new_row = center_row - rotated_piece->height / 2;
	int new_col = center_col - rotated_piece->width / 2;

	// Try all possible wall kick positions
	for (int i = 0; i < 10; i++) {
		int attempt_row = new_row + wall_kick_attempts[i][0];
		int attempt_col = new_col + wall_kick_attempts[i][1];

		if (validate_piece_at_position(grid, rotated_piece, attempt_row, attempt_col)) {
			rotated_piece->row_pos = attempt_row;
			rotated_piece->col_pos = attempt_col;
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

void clear_grid(Grid* grid) {
	for (int i = 0; i < grid->height; i++)
	{
		for (int j = 0; j < grid->width; j++)
		{
			grid->cells[i][j].piece = NULL;
			grid->cells[i][j].shadow = false;
			grid->cells[i][j].locked = false;
		}
	}
	clear_x_cells(grid);
	clear_dynamic_array(grid->locked_pieces);
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

	bool height_warning = grid->is_game_board && is_near_height_limit(grid);

	for (int i = 0; i < grid->height; i++) {
		for (int j = 0; j < grid->width; j++) {
			SDL_Rect cell_rect = {
				j * cell_width + origin_x + border_width,
				i * cell_width + origin_y + border_width,
				cell_width,
				cell_width
			};
			if (i < 2 && height_warning) {
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
				SDL_RenderFillRect(renderer, &cell_rect);
			}
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
			// For debugging, normally this would be an illegal state. Helpful to visualize if row clearing messes up.
			if (grid->cells[i][j].locked && !grid->cells[i][j].piece) {
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128);
				SDL_RenderFillRect(renderer, &cell_rect);
			}

			if (grid->cells[i][j].x) {
				SDL_SetRenderDrawColor(renderer, 210, 0, 0, 255);
				// Top left to bottom right
				// To draw a thicker line, draw multiple lines with a 1 pixel offset
                for (int k = -3; k < 4; k++) {
					int x = MAX(cell_rect.x, cell_rect.x + k) + 1;
					int y = MAX(cell_rect.y, cell_rect.y - k) + 1;
					int x2 = MIN(cell_rect.x + cell_width, cell_rect.x + cell_width + k) - 2;
					int y2 = MIN(cell_rect.y + cell_width, cell_rect.y + cell_width - k) - 2;
					SDL_RenderDrawLine(renderer, x, y, x2, y2);
				}

				// Bottom left to top right
				for (int k = -3; k < 4; k++) {
					int x = MAX(cell_rect.x, cell_rect.x + k) + 1;
					int y = MIN(cell_rect.y + cell_width, cell_rect.y + cell_width + k) - 2;
					int x2 = MIN(cell_rect.x + cell_width, cell_rect.x + cell_width + k) - 2;
					int y2 = MAX(cell_rect.y, cell_rect.y + k) + 1;
					SDL_RenderDrawLine(renderer, x, y, x2, y2);
				}
			}
		}
	}
}

static void clear_x_cells(Grid* grid) {
	for (int i = 0; i < grid->height; i++) {
		for (int j = 0; j < grid->width; j++) {
			grid->cells[i][j].x = false;
		}
	}
}

void mark_x_cells(Grid* grid, Piece* piece) {
	for (int i = 0; i < piece->height; i++) {
		for (int j = 0; j < piece->width; j++) {
			if (piece->shape[i * piece->width + j]) {
				grid->cells[piece->row_pos + i][piece->col_pos + j].x = true;
			}
		}
	}
}

static bool insert_piece(Grid* grid, Piece* piece, bool lock) {
	if (!validate_piece_position(grid, piece)) {
		return false;
	}

	int row = piece->row_pos;
	int col = piece->col_pos;

	Piece* piece_copy = piece; // Default to original unless locking
	if (lock && !dynamic_array_contains(grid->locked_pieces, piece)) {
		piece_copy = copy_piece(piece); // Locked implies the Grid is now meant to own the piece. Copy it so Game can destroy its copy later.
		add_to_dynamic_array(grid->locked_pieces, piece_copy);
	}

	// Draw every cell of the piece to it's corresponding cell in the grid
	for (int i = 0; i < piece->height; i++) {
		for (int j = 0; j < piece->width; j++) {
			if (piece->shape[i * piece->width + j]) {
				grid->cells[row + i][col + j].piece = piece_copy;
				grid->cells[row + i][col + j].locked = lock;
			}
		}
	}

	return true;
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
		cells[i].x = false;
	}
}

static void test_print(Grid* grid) {
	// Print the state of the board for debugging
	for (int i = 0; i < grid->height; i++) {
		for (int j = 0; j < grid->width; j++) {
			if (grid->cells[i][j].piece && grid->cells[i][j].locked) {
				printf("X");
			}
			else if (grid->cells[i][j].piece && !grid->cells[i][j].locked) {
				printf("P");
			}
			else if (!grid->cells[i][j].piece && grid->cells[i][j].locked) {
				printf("L");
			}
			else {
				printf("0");
			}
		}
		printf("\n");
	}
}

static void test_print_file(Grid* grid, const char* label, int index) {
	FILE* file = fopen("Debugging.txt", "a"); // Open in append mode to keep history
	if (!file) {
		fprintf(stderr, "Error: Failed to open file for writing\n");
		return;
	}

	fprintf(file, "Grid State: %s at index %d\n", label, index);
	for (int i = 0; i < grid->height; i++) {
		for (int j = 0; j < grid->width; j++) {
			if (grid->cells[i][j].piece && grid->cells[i][j].locked) {
				fputc('X', file);
			}
			else if (grid->cells[i][j].piece && !grid->cells[i][j].locked) {
				fputc('P', file);
			}
			else if (!grid->cells[i][j].piece && grid->cells[i][j].locked) {
				fputc('L', file);
			}
			else {
				fputc('0', file);
			}
		}
		fputc('\n', file);
	}
	fprintf(file, "\n"); // Add a blank line for readability

	fclose(file);
}

// Need to do a recursive check if dropping a piece causes a new row to be full
int check_and_clear_full_rows(Grid* grid) {
	int cleared_rows = 0;
	for (int row = 0; row < grid->height; row++) {
		bool row_full = true;
		for (int col = 0; col < grid->width; col++) {
			if (!grid->cells[row][col].locked) {
				row_full = false;
				break;
			}
		}
		if (row_full) {
			DynamicArray* pieces_to_split = create_dynamic_array(10, NULL);
			// Clear the row
			for (int col = 0; col < grid->width; col++) {
				if (grid->cells[row][col].piece) {
					Piece* piece = grid->cells[row][col].piece;

					// Convert global grid coordinates to local piece coordinates
					int local_row = row - piece->row_pos;
					int local_col = col - piece->col_pos;

					// Delete the part of the piece that is in the row
					if (local_row >= 0 && local_row < piece->height && local_col >= 0 && local_col < piece->width) {
						piece->shape[local_row * piece->width + local_col] = false;
					}

					// Check if the piece spans both above and below
					bool has_above = false;
					bool has_below = false;

					// Check the entire row above
					if (local_row > 0) {
						for (int c = 0; c < piece->width; c++) {
							if (piece->shape[(local_row - 1) * piece->width + c]) {
								has_above = true;
								break; // Stop early if any block is found above
							}
						}
					}

					// Check the entire row below
					if (local_row < piece->height - 1) {
						for (int c = 0; c < piece->width; c++) {
							if (piece->shape[(local_row + 1) * piece->width + c]) {
								has_below = true;
								break; // Stop early if any block is found below
							}
						}
					}

					if (has_above && has_below && !dynamic_array_contains(pieces_to_split, piece)) {
						add_to_dynamic_array(pieces_to_split, piece);
					}

					grid->cells[row][col].piece = NULL;
					grid->cells[row][col].locked = false;
				}
			}
			for (int j = 0; j < pieces_to_split->size; j++) {
				// Piece is split! Create two new pieces
				Piece* piece = get_from_dynamic_array(pieces_to_split, j);
				int local_row = row - piece->row_pos;
				Piece* top_half = copy_piece_region(piece, 0, 0, local_row, piece->width);
				Piece* bottom_half = copy_piece_region(piece, local_row + 1, 0, piece->height - local_row - 1, piece->width);

				top_half->color = (SDL_Color){ 60, 128, 60, SDL_ALPHA_OPAQUE };
				bottom_half->color = (SDL_Color){ 128, 60, 128, SDL_ALPHA_OPAQUE };

				top_half->row_pos = piece->row_pos;
				top_half->col_pos = piece->col_pos;
				bottom_half->row_pos = piece->row_pos + local_row + 1;
				bottom_half->col_pos = piece->col_pos;

				// Reassign grid cell pointers to the new pieces
				for (int r = 0; r < top_half->height; r++) {
					for (int c = 0; c < top_half->width; c++) {
						if (top_half->shape[r * top_half->width + c]) {
							grid->cells[top_half->row_pos + r][piece->col_pos + c].piece = top_half;
						}
					}
				}

				for (int r = 0; r < bottom_half->height; r++) {
					for (int c = 0; c < bottom_half->width; c++) {
						if (bottom_half->shape[r * bottom_half->width + c]) {
							grid->cells[bottom_half->row_pos + r][piece->col_pos + c].piece = bottom_half;
						}
					}
				}

				add_to_dynamic_array(grid->locked_pieces, top_half);
				add_to_dynamic_array(grid->locked_pieces, bottom_half);

				// Remove old piece from tracking
				remove_from_dynamic_array(grid->locked_pieces, piece);
				destroy_piece(piece);
			}
			destroy_dynamic_array(pieces_to_split);
			cleared_rows++;
		}
	}

	remove_empty_pieces(grid);
	return cleared_rows;
}

static void clear_piece_pointers(Grid* grid, Piece* piece) {
	for (int k = 0; k < piece->height; k++) {
		for (int l = 0; l < piece->width; l++) {
			if (piece->shape[k * piece->width + l]) {
				SDL_assert(grid->cells[k + piece->row_pos][l + piece->col_pos].piece == piece);
				grid->cells[k + piece->row_pos][l + piece->col_pos].piece = NULL;
			}
		}
	}
}

static void set_lock(Piece* piece, Grid* grid, bool lock) {
	for (int l = 0; l < piece->height; l++) {
		for (int m = 0; m < piece->width; m++) {
			if (piece->shape[l * piece->width + m]) {
				grid->cells[l + piece->row_pos][m + piece->col_pos].locked = lock;
			}
		}
	}
}

static bool is_row_empty(Grid* grid, int row) {
	for (int col = 0; col < grid->width; col++) {
		if (grid->cells[row][col].piece) {
			return false;
		}
	}
	return true;
}

void drop_all_pieces(Grid* grid) {
	int num_empty_rows = 0;
	int index = 0;
	DynamicArray* pieces_to_drop = create_dynamic_array(10, NULL);
	for (int row = grid->height - 1; row >= 0; row--) {
		// Gather pieces to drop
		for (int col = 0; col < grid->width; col++) {
			if (grid->cells[row][col].piece) {
				Piece* piece = grid->cells[row][col].piece;
				if (!dynamic_array_contains(pieces_to_drop, piece)) {
					add_to_dynamic_array(pieces_to_drop, piece);
				}
			}
		}
		if (is_row_empty(grid, row) || row == 0) {
			// Clear these pieces and set the new location to piece property
			for (int i = index; i < pieces_to_drop->size; i++) {
				Piece* piece = get_from_dynamic_array(pieces_to_drop, i);
				// Drop the piece down by the number of empty rows
				set_lock(piece, grid, false);
				clear_piece_pointers(grid, piece);
				piece->row_pos += num_empty_rows;
				remove_from_dynamic_array(grid->locked_pieces, piece);

			}
			num_empty_rows++;
			index = pieces_to_drop->size;
		}
	}

	// Insert pieces at new location
	for (int i = 0; i < pieces_to_drop->size; i++) {
		Piece* piece = get_from_dynamic_array(pieces_to_drop, i);
		insert_piece(grid, piece, true);
	}

	destroy_dynamic_array(pieces_to_drop);


	// Now go over each piece and see if it can drop further
	for (int i = 0; i < grid->locked_pieces->size; i++) {
		Piece* piece = get_from_dynamic_array(grid->locked_pieces, i);
		set_lock(piece, grid, false);
		clear_piece_pointers(grid, piece);
		drop_piece_on_grid(grid, piece, true);
	}

	// For each piece assert that it can't drop further
	for (int i = 0; i < grid->locked_pieces->size; i++) {
		Piece* piece = get_from_dynamic_array(grid->locked_pieces, i);
		SDL_assert(!validate_piece_at_position(grid, piece, piece->row_pos + 1, piece->col_pos));
	}

}


static void remove_empty_pieces(Grid* grid) {
	for (int i = 0; i < grid->locked_pieces->size; i++) {
		Piece* piece = (Piece*)grid->locked_pieces->items[i];

		if (is_piece_empty(piece)) {
			remove_from_dynamic_array(grid->locked_pieces, piece);
			destroy_piece(piece);
			//printf("Removed empty piece\n");
			i--; // Adjust index since we removed an item
		}
	}
}
