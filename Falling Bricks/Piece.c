#include "Piece.h"
#include <stdio.h>

static bool calloc_failed(bool* shape);

Piece* create_piece(enum PieceType type) {
	Piece* piece = malloc(sizeof(Piece));
	if (!piece) {
		fprintf(stderr, "Error: Failed to allocate memory for Piece\n");
		return NULL;
	}
	SDL_assert(type >= 0 && type < 7);
	piece->row_pos = 0;
	piece->col_pos = 0;
	piece->type = type;
	switch (type) {
	case LINE:
		// 1 1 1 1
		piece->width = 4;
		piece->height = 1;
		piece->shape = calloc(piece->height * piece->width, sizeof(bool));
		if (calloc_failed(piece->shape)) { free(piece); return NULL; }
		piece->shape[0] = true;
		piece->shape[1] = true;
		piece->shape[2] = true;
		piece->shape[3] = true;
		piece->color = (SDL_Color){ 0, 255, 255, SDL_ALPHA_OPAQUE };
		break;
	case L:
		// 1 0 0
		// 1 1 1
		piece->width = 3;
		piece->height = 2;
		piece->shape = calloc(piece->height * piece->width, sizeof(bool));
		if (calloc_failed(piece->shape)) { free(piece); return NULL; }
		piece->shape[0] = true;
		piece->shape[3] = true;
		piece->shape[4] = true;
		piece->shape[5] = true;
		piece->color = (SDL_Color){ 255, 0, 255, SDL_ALPHA_OPAQUE };
		break;
	case LR:
		// 0 0 1
		// 1 1 1
		piece->width = 3;
		piece->height = 2;
		piece->shape = calloc(piece->height * piece->width, sizeof(bool));
		if (calloc_failed(piece->shape)) { free(piece); return NULL; }
		piece->shape[2] = true;
		piece->shape[3] = true;
		piece->shape[4] = true;
		piece->shape[5] = true;
		piece->color = (SDL_Color){ 255, 255, 0, SDL_ALPHA_OPAQUE };
		break;
	case S:
		// 1 1
		// 1 1
		piece->width = 2;
		piece->height = 2;
		piece->shape = calloc(piece->height * piece->width, sizeof(bool));
		if (calloc_failed(piece->shape)) { free(piece); return NULL; }
		piece->shape[0] = true;
		piece->shape[1] = true;
		piece->shape[2] = true;
		piece->shape[3] = true;
		piece->color = (SDL_Color){ 0, 255, 0, SDL_ALPHA_OPAQUE };
		break;
	case Z:
		// 1 1 0
		// 0 1 1
		piece->width = 3;
		piece->height = 2;
		piece->shape = calloc(piece->height * piece->width, sizeof(bool));
		if (calloc_failed(piece->shape)) { free(piece); return NULL; }
		piece->shape[0] = true;
		piece->shape[1] = true;
		piece->shape[4] = true;
		piece->shape[5] = true;
		piece->color = (SDL_Color){ 255, 0, 0, SDL_ALPHA_OPAQUE };
		break;
	case ZR:
		// 0 1 1
		// 1 1 0
		piece->width = 3;
		piece->height = 2;
		piece->shape = calloc(piece->height * piece->width, sizeof(bool));
		if (calloc_failed(piece->shape)) { free(piece); return NULL; }
		piece->shape[1] = true;
		piece->shape[2] = true;
		piece->shape[3] = true;
		piece->shape[4] = true;
		piece->color = (SDL_Color){ 0, 0, 255, SDL_ALPHA_OPAQUE };
		break;
	case T:
		// 0 1 0
		// 1 1 1
		piece->width = 3;
		piece->height = 2;
		piece->shape = calloc(piece->height * piece->width, sizeof(bool));
		if (calloc_failed(piece->shape)) { free(piece); return NULL; }
		piece->shape[1] = true;
		piece->shape[3] = true;
		piece->shape[4] = true;
		piece->shape[5] = true;
		piece->color = (SDL_Color){ 255, 113, 0, SDL_ALPHA_OPAQUE };
		break;
	default:
		fprintf(stderr, "Error: Invalid Piece Type\n");
		free(piece);
		return NULL;
	}
	return piece;
}

Piece* rotate_piece(const Piece* piece, bool clockwise) {
	Piece* rotated_piece = malloc(sizeof(Piece));

	if (!rotated_piece) {
		fprintf(stderr, "Error: Failed to allocate memory for rotated Piece\n");
		return NULL;
	}

	int new_width = piece->height;
	int new_height = piece->width;
	rotated_piece->width = new_width;
	rotated_piece->height = new_height;
	rotated_piece->color = piece->color;
	rotated_piece->shape = calloc(new_width * new_height, sizeof(bool));
	if (calloc_failed(rotated_piece->shape)) { free(piece); return NULL; }

	for (int i = 0; i < new_width; i++) {
		for (int j = 0; j < new_height; j++) {
			int col = clockwise ? new_width - 1 - i : i; // clockwise or counter-clockwise. In other words, start from the last col or first col
			int row = clockwise ? j : new_height - 1 - j; // clockwise or counter-clockwise. In other words, start from the first row or last row
			rotated_piece->shape[row * new_width + col] = piece->shape[i * piece->width + j];
			// shape[j][new_width - 1 - i] = piece->shape[i][j];
		}
	}

	return rotated_piece;
}

bool is_piece_empty(const Piece* piece) {
	for (int i = 0; i < piece->height * piece->width; i++) {
		if (piece->shape[i]) {
			return false;
		}
	}
	return true;
}

Piece* copy_piece(const Piece* piece) {
	Piece* new_piece = malloc(sizeof(Piece));
	if (!new_piece) {
		fprintf(stderr, "Error: Failed to allocate memory for copied Piece\n");
		return NULL;
	}
	new_piece->width = piece->width;
	new_piece->height = piece->height;
	new_piece->row_pos = piece->row_pos;
	new_piece->col_pos = piece->col_pos;
	new_piece->type = piece->type;
	new_piece->color = piece->color;
	new_piece->shape = calloc(new_piece->height * new_piece->width, sizeof(bool));
	if (calloc_failed(new_piece->shape)) { free(new_piece); return NULL; }
	for (int i = 0; i < new_piece->height * new_piece->width; i++) {
		new_piece->shape[i] = piece->shape[i];
	}
	return new_piece;
}

Piece* copy_piece_region(Piece* original, int start_row, int start_col, int new_height, int new_width) {
	Piece* new_piece = malloc(sizeof(Piece));
	if (!new_piece) {
		fprintf(stderr, "Error: Failed to allocate memory for copied Piece region\n");
		return NULL;
	}
	new_piece->width = new_width;
	new_piece->height = new_height;
	new_piece->color = original->color;
	new_piece->type = original->type;
	new_piece->row_pos = 0;
	new_piece->col_pos = 0;
	new_piece->shape = calloc(new_width * new_height, sizeof(bool));
	if (calloc_failed(new_piece->shape)) { free(new_piece); return NULL; }

	for (int i = 0; i < new_height; i++) {
		for (int j = 0; j < new_width; j++) {
			new_piece->shape[i * new_width + j] = original->shape[(i + start_row) * original->width + (j + start_col)];
		}
	}

	return new_piece;
}

void destroy_piece(Piece* piece) {
	if (piece) {
		free(piece->shape);
		free(piece);
	}
}

static bool calloc_failed(bool* shape)
{
	if (!shape)
	{
		fprintf(stderr, "Error: Failed to allocate memory for Piece Shape\n");
		return true;
	}
	return false;
}