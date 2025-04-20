#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Constants.h"
#include "Grid.h"
#include "Piece.h"
#include "Queue.h"
#include "DynamicArray.h"
#include "Menu.h"
#include "Game.h"

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

extern TTF_Font* font;

int last_frame_time = 0;

int last_drop_time = 0;

bool round_active = false;
bool dropping_pieces = false;

Piece* player_piece = NULL;
Grid* game_board = NULL;

Grid* queue_grid = NULL;
Queue* next_pieces = NULL;

SDL_Renderer* debug_renderer = NULL;

struct TitleMenu* title_menu = NULL;
struct GameOverMenu* game_over_menu = NULL;

typedef enum {
	GAME_STATE_MENU,
	GAME_OVER_MENU,
	GAME_STATE_PLAYING
} GameState;

typedef enum {
	FOURTY_LINES,
	BLITZ,
	ENDLESS
} GameMode;

struct Game {
	GameState current_state;
	GameMode current_mode;
} game = { 0 };


struct Flags {
	bool move_player_down;
	bool move_player_left;
	bool move_player_right;
	bool rotate_player;
	bool drop_player;
	bool pause;
} flags = { 0 };

static Piece* create_random_piece() {
	int random_piece = rand() % 7;
	return create_piece(random_piece);
}

static void screenshot_debug() {
	char buffer[100];
	sprintf_s(buffer, sizeof(buffer), "%d.bmp", (int)SDL_GetTicks());
	SDL_Surface* sshot = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_RenderReadPixels(debug_renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
	SDL_SaveBMP(sshot, buffer);
	SDL_FreeSurface(sshot);
}

void start_game() {
	game.current_state = GAME_STATE_PLAYING;
	round_active = true;
	last_drop_time = SDL_GetTicks();
	player_piece = create_random_piece();
	player_piece->row_pos = 0;
	player_piece->col_pos = game_board->width / 2 - player_piece->width / 2;
}

void start_fourty_lines() {
	game.current_mode = FOURTY_LINES;
	start_game();
}

void start_blitz() {
	game.current_mode = BLITZ;
	start_game();
}

void start_endless() {
	game.current_mode = ENDLESS;
	start_game();
}

void main_menu() {
	game.current_state = GAME_STATE_MENU;
	round_active = false;
}

void send_quit() {
	SDL_PushEvent(&(SDL_Event) { .type = SDL_QUIT });
}

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

bool setup() {

	title_menu = create_title_menu((ButtonCallback[]) {
		start_fourty_lines,
		start_blitz,
		start_endless,
		send_quit
	}, font);

	game_over_menu = create_game_over_menu((ButtonCallback[]) {
		main_menu,
		send_quit
	}, font);

	game_board = create_grid(BOARD_WIDTH, BOARD_HEIGHT, true);

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

	if (!game_board || !title_menu || !game_over_menu)
	{
		fprintf(stderr, "Fatal Error during setup\n"); 
		return false;
	}
	
	return true;
}

void cleanup() {
	destroy_piece(player_piece);
	destroy_grid(game_board);
	destroy_queue(next_pieces);
	destroy_title_menu(title_menu);
	destroy_game_over_menu(game_over_menu);
}

void process_input(bool* running) {
	SDL_Event event;
	SDL_PollEvent(&event);

	if (event.type == SDL_QUIT) {
		*running = false;
		return;
	}

	if (game.current_state == GAME_STATE_MENU) {
		handle_title_menu_events(title_menu, event);
		return;
	}
	if (game.current_state == GAME_OVER_MENU) {
		handle_game_over_menu_events(game_over_menu, event);
		return;
	}

	if (event.type == SDL_KEYDOWN) {
		int key = event.key.keysym.sym;
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

	if (game.current_state != GAME_STATE_PLAYING) {
		return;
	}

	if (flags.pause) {
		return;
	}

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
		if (dropping_pieces) {
			SDL_Delay(1000);
			drop_all_pieces(game_board);
			if (check_and_clear_full_rows(game_board) == 0) {
				dropping_pieces = false;
			}
			last_drop_time = SDL_GetTicks();
			return;
		}
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
			destroy_piece(player_piece);
			player_piece = NULL;
			printf("Game Over\n");
			game.current_state = GAME_OVER_MENU;
			return;
		}

		if (lock_piece) {
			destroy_piece(player_piece);
			player_piece = dequeue(next_pieces);
			player_piece->row_pos = 0;
			player_piece->col_pos = game_board->width / 2 - player_piece->width / 2;

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

			// Check for full rows
			if (check_and_clear_full_rows(game_board) > 0) {
				dropping_pieces = true;
				return;
			}
			
		}
	}
}

void render(SDL_Renderer* renderer) {
	debug_renderer = renderer;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	//SDL_Rect my_rect = { 600 * scale_factor, 300 * scale_factor, 100 * scale_factor, 100 * scale_factor };


	//SDL_Surface* screen = SDL_GetWindowSurface(window);

	SDL_Window* window = SDL_GetWindowFromID(1);
	int window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);
	float scale_factor = MIN((float)window_width / WINDOW_WIDTH, (float)window_height / WINDOW_HEIGHT);

	if (game.current_state == GAME_STATE_MENU) {
		title_menu->scale_factor = scale_factor;
		draw_title_menu(title_menu, renderer);
	}
	else if (game.current_state == GAME_OVER_MENU) {
		game_over_menu->scale_factor = scale_factor;
		draw_game_over_menu(game_over_menu, renderer);
	}
	else {
		int cell_width = 32 * scale_factor;
		int board_x = (float)WINDOW_WIDTH / 2 - (float)cell_width * BOARD_WIDTH / 2;
		board_x *= scale_factor;
		int board_y = 50;
		draw_grid(game_board, board_x, board_y, cell_width, true, renderer);
		draw_grid(queue_grid, 50 * scale_factor + game_board->width * cell_width + board_x, board_y, cell_width, true, renderer);
	}

	

	SDL_RenderPresent(renderer);
}