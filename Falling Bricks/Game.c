#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Constants.h"
#include "Grid.h"
#include "Piece.h"
#include "Queue.h"
#include "Game.h"

int last_frame_time = 0;

int last_drop_time = 0;

float scale_factor = 1;

bool round_active = false;

Piece* player_piece = NULL;

Grid* game_board = NULL;

Grid* queue_grid = NULL;
Queue* next_pieces = NULL;

struct Flags {
	bool move_player_down;
	bool move_player_left;
	bool move_player_right;
	bool rotate_player;
	bool drop_player;
	bool pause;
} flags = { 0 };

static bool move_player_left() {
	if (validate_piece_at_position(game_board, player_piece, player_piece->row_pos, player_piece->col_pos - 1)) {
		player_piece->col_pos--;
		return true;
	}
	return false;
}

static bool move_player_right() {
	if (validate_piece_at_position(game_board, player_piece, player_piece->row_pos, player_piece->col_pos + 1)) {
		player_piece->col_pos++;
		return true;
	}
	return false;
}

static bool move_player_down() {
	if (validate_piece_at_position(game_board, player_piece, player_piece->row_pos + 1, player_piece->col_pos)) {
		player_piece->row_pos++;
		return true;
	}
	return false;
}

static Piece* create_random_piece() {
	int random_piece = rand() % 7;
	return create_piece(random_piece);
}

bool setup() {

	game_board = create_grid(10, 20, true);
	player_piece = create_piece(T);

	queue_grid = create_grid(6, 19, true);
	queue_grid->show_grid_lines = false;

	next_pieces = create_queue(destroy_piece);

	for (int i = 0; i < 6; i++) {
		Piece* new_piece = create_random_piece();
		new_piece->row_pos = 3 * i + 1;
		new_piece->col_pos = 1;
		enqueue(next_pieces, new_piece);
		add_piece_to_grid(queue_grid, new_piece, true, false);
	}

	if (!player_piece || !game_board)
	{
		fprintf(stderr, "Fatal Error during setup\n"); 
		return false;
	}

	round_active = true;

	return true;
}

void cleanup() {
	destroy_piece(player_piece);
	destroy_grid(game_board);
	destroy_queue(next_pieces);
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
			flags.rotate_player = true;
		}
		else if (key == SDLK_DOWN) {
			flags.move_player_down = true;
		}
		else if (key == SDLK_LEFT) {
			flags.move_player_left = true;
		}
		else if (key == SDLK_RIGHT) {
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

	//ball.x += 70 * delta_time;
	//ball.y += 50 * delta_time;
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

	if (round_active) {
		if (SDL_GetTicks() - last_drop_time >= 1000) {
			last_drop_time = SDL_GetTicks();
			flags.move_player_down = true;
		}

		if (flags.move_player_down) {
			lock_piece = !move_player_down();
			flags.move_player_down = false;
			flags.rotate_player = false;
		}
		if (flags.move_player_left) {
			move_player_left();
			flags.move_player_left = false;
		}
		if (flags.move_player_right) {
			move_player_right();
			flags.move_player_right = false;
		}
		if (flags.drop_player) {
			drop = true;
			lock_piece = true;
			flags.drop_player = false;
		}
		if (flags.rotate_player) {
			Piece* rotated_piece = try_rotate_piece(game_board, player_piece);
			if (rotated_piece) {
				destroy_piece(player_piece);
				// Update to rotated piece and new position after rotation
				player_piece = rotated_piece;
			}
			flags.rotate_player = false;
		}

		clear_unlocked_cells(game_board);
		bool piece_added = add_piece_to_grid(game_board, player_piece, lock_piece, drop);

		if (!piece_added) {
			round_active = false;
			printf("Game Over\n");
		}

		if (lock_piece) {
			// Check for full rows
			clear_full_rows(game_board);
			destroy_piece(player_piece);
			player_piece = dequeue(next_pieces);
			player_piece->row_pos = 0;
			player_piece->col_pos = 0;

			enqueue(next_pieces, create_random_piece());
			// Update queue grid
			clear_grid(queue_grid);
			Node* current = next_pieces->front;
			for (int i = 0; i < next_pieces->size; i++) {
				Piece* next_piece = (Piece*)current->data;
				next_piece->row_pos = 3 * i + 1;
				next_piece->col_pos = 1;
				add_piece_to_grid(queue_grid, next_piece, true, false);
				current = current->next;
			}
		}
	}
}

void render(SDL_Renderer* renderer) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	//SDL_Rect my_rect = { 600 * scale_factor, 300 * scale_factor, 100 * scale_factor, 100 * scale_factor };
	//SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	//SDL_RenderDrawRect(renderer, &my_rect);

	//SDL_Surface* screen = SDL_GetWindowSurface(window);

	// Draw grid
	SDL_Color cell_color = { 0, 177, 0, 255 };


	draw_grid(game_board, 50, 50, 32, true, renderer);
	draw_grid(queue_grid, 50 + game_board->width * 32 + 50, 50, 32, true, renderer);

	SDL_RenderPresent(renderer);
}