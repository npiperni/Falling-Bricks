#include <stdio.h>
#include <stdbool.h>

#include "Piece.h"

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
		piece->color = (SDL_Color){ 0, 255, 255, 255 };
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
		piece->color = (SDL_Color){ 0, 255, 255, 255 };
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
		piece->color = (SDL_Color){ 0, 255, 255, 255 };
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
		piece->color = (SDL_Color){ 0, 255, 0, 255 };
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
		piece->color = (SDL_Color){ 255, 0, 0, 255 };
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
		piece->color = (SDL_Color){ 255, 0, 0, 255 };
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
		piece->color = (SDL_Color){ 128, 0, 128, 255 };
		break;
	default:
		fprintf(stderr, "Error: Invalid Piece Type\n");
		free(piece);
		return NULL;
	}
	return piece;
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