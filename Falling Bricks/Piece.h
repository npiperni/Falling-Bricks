#pragma once

#include <stdbool.h>
#include <SDL.h>

enum PieceType {
	LINE = 0,
	L = 1,
	LR = 2,
	S = 3,
	Z = 4,
	ZR = 5,
	T = 6
};

typedef struct {
    bool* shape;
    int width;
	int height;
	int row_pos;
	int col_pos;
	enum PieceType type;
    SDL_Color color;
} Piece;


Piece* create_piece(enum PieceType type);

Piece* rotate_piece(const Piece* piece);

bool is_piece_empty(const Piece* piece);

Piece* copy_piece(const Piece* piece);

void destroy_piece(Piece* piece);