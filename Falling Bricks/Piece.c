#include "Piece.h"
#include <stdio.h>

static bool calloc_failed(bool* shape);

Piece* create_piece(enum PieceType type) {
	Piece* piece = malloc(sizeof(Piece));
	if (!piece) {
		fprintf(stderr, "Error: Failed to allocate memory for Piece\n");
		return NULL;
	}

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

Piece* rotate_piece(const Piece* piece) {
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
			rotated_piece->shape[j * new_width + (new_width - 1 - i)] = piece->shape[i * piece->width + j];
			// shape[j][new_width - 1 - i] = piece->shape[i][j];
		}
	}

	return rotated_piece;
}

void destroy_piece(Piece* piece) {
	free(piece->shape);
	free(piece);
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