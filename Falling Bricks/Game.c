#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Constants.h"
#include "Grid.h"
#include "Piece.h"
#include "Game.h"

int last_frame_time = 0;

int last_drop_time = 0;

float scale_factor = 1;

struct ball {
	float x;
	float y;
	float width;
	float height;
} ball;

struct player {
	int row;
	int col;
} player;

Grid* grid = NULL;

Piece* piece = NULL;

struct Flags {
	bool move_player_down;
	bool move_player_left;
	bool move_player_right;
	bool rotate_player;
	bool drop_player;
	bool pause;
} flags = { 0 };

static bool move_player_left(Grid* grid, Piece* piece) {
	if (validate_piece_position(grid, piece, player.row, player.col - 1)) {
		player.col--;
		return true;
	}
	return false;
}

static bool move_player_right(Grid* grid, Piece* piece) {
	if (validate_piece_position(grid, piece, player.row, player.col + 1)) {
		player.col++;
		return true;
	}
	return false;
}

static bool move_player_down(Grid* grid, Piece* piece) {
	if (validate_piece_position(grid, piece, player.row + 1, player.col)) {
		player.row++;
		return true;
	}
	return false;
}

static Piece* create_random_piece() {
	int random_piece = rand() % 7;
	return create_piece(random_piece);
}

bool setup() {
	ball.x = 20;
	ball.y = 20;
	ball.width = 15;
	ball.height = 15;

	player.row = 0;
	player.col = 0;

	grid = create_grid(10, 20, true);
	piece = create_piece(LINE);
	if (!piece || !grid)
	{
		fprintf(stderr, "Fatal Error during setup\n"); 
		return false;
	}
	return true;
}

void cleanup() {
	destroy_piece(piece);
	destroy_grid(grid);
}

void process_input(bool* running) {
	SDL_Event event;
	SDL_PollEvent(&event);

	int key = 0;
	switch (event.type) {
	case SDL_QUIT:
		*running = false;
		break;
	case SDL_KEYDOWN:
		key = event.key.keysym.sym;
		if (key == SDLK_ESCAPE) {
			*running = false;
		}
		else if (key == SDLK_p) {
			flags.pause = !flags.pause;
		}
		else if (key == SDLK_UP) {
			ball.y -= 50;
			flags.rotate_player = true;
		}
		else if (key == SDLK_DOWN) {
			ball.y += 50;
			flags.move_player_down = true;
		}
		else if (key == SDLK_LEFT) {
			ball.x -= 50;
			flags.move_player_left = true;
		}
		else if (key == SDLK_RIGHT) {
			ball.x += 50;
			flags.move_player_right = true;
		}
		else if (key == SDLK_SPACE) {
			flags.drop_player = true;
		}
		break;
	case SDL_MOUSEMOTION:
		//printf("Mouse moved.\n");
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == SDL_BUTTON_LEFT)
		{
			printf("Clicked mouse button 1\n");
		}
		break;
	}
}

void update() {
	//while (!SDL_TICKS_PASSED(SDL_GetTicks(), last_frame_time + FRAME_TARGET_TIME));

	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
	last_frame_time = SDL_GetTicks();

	if (flags.pause) {
		return;
	}

	ball.x += 70 * delta_time;
	ball.y += 50 * delta_time;
	//printf("%d\n", SDL_GetTicks());

	//int current_width, current_height;
	//SDL_GetWindowSize(window, &current_width, &current_height);
	//scale_factor = MIN((float)current_width / WINDOW_WIDTH, (float)current_height / WINDOW_HEIGHT);
	//printf("%f\n", scale_factor);
	//int displayIndex = SDL_GetWindowDisplayIndex(window);
	//SDL_DisplayMode display_mode;
	//SDL_GetCurrentDisplayMode(displayIndex, &display_mode);
	//printf("%d %d\n", display_mode.w, display_mode.h);

	bool lock_piece = false;
	bool drop = false;

	if (SDL_GetTicks() - last_drop_time >= 1000) {
		last_drop_time = SDL_GetTicks();
		flags.move_player_down = true;
	}

	if (flags.move_player_down) {
		lock_piece = !move_player_down(grid, piece);
		flags.move_player_down = false;
		flags.rotate_player = false;
	}
	if (flags.move_player_left) {
		move_player_left(grid, piece);
		flags.move_player_left = false;
	}
	if (flags.move_player_right) {
		move_player_right(grid, piece);
		flags.move_player_right = false;
	}
	if (flags.drop_player) {
		drop = true;
		flags.drop_player = false;
	}
	if (flags.rotate_player) {
		Piece* rotated_piece = try_rotate_piece(grid, piece, &player.row, &player.col);
		if (rotated_piece) {
			destroy_piece(piece);
			// Update to rotated piece and new position after rotation
			piece = rotated_piece;
		}
		flags.rotate_player = false;
	}

	clear_unlocked_cells(grid);
	add_piece_to_grid(grid, piece, player.row, player.col, lock_piece, drop);

	if (lock_piece || drop) {
		//destroy_piece(piece);
		piece = create_random_piece();
		player.row = 0;
		player.col = 0;
	}
}

void render(SDL_Renderer* renderer) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	// Draw rectangle
	SDL_Rect ball_rect = {
		(int)ball.x,
		(int)ball.y,
		(int)ball.width,
		(int)ball.height
	};

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);


	SDL_RenderFillRect(renderer, &ball_rect);


	SDL_Rect my_rect = { 600 * scale_factor, 300 * scale_factor, 100 * scale_factor, 100 * scale_factor };
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &my_rect);

	//SDL_Surface* screen = SDL_GetWindowSurface(window);

	// Draw grid
	SDL_Color cell_color = { 0, 177, 0, 255 };


	draw_grid(grid, renderer);

	SDL_RenderPresent(renderer);
}