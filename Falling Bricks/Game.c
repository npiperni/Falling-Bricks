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
#include "Label.h"
#include "Game.h"

extern TTF_Font* button_font;
extern TTF_Font* title_font;
extern TTF_Font* label_font;
extern TTF_Font* label_font_small;

int last_frame_time = 0;

int last_drop_time = 0;

float scale_factor = 1.0f;

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
	GAME_STATE_PLAYING,
	GAME_STATE_COUNTDOWN
} GameState;

typedef enum {
	FOURTY_LINES,
	BLITZ,
	ENDLESS
} GameMode;

struct Game {
	GameState current_state;
	GameMode current_mode;
	int score;
	int total_lines_cleared;
	int level;
	Uint32 start_time;
	Uint32 pause_start_time;
	Uint32 total_paused_time;
	Uint32 elapsed_time;
	int current_lines_cleared;
	char main_label[20];
	Uint32 label_display_start_time;
	int countdown;
} game = { 0 };

struct Flags {
	bool move_player_down;
	bool move_player_left;
	bool move_player_right;
	bool rotate_player;
	bool drop_player;
	bool pause;
	bool clearing_rows;
	bool dropping_pieces;
} flags = { 0 };

static Piece* create_random_piece() {
	int random_piece = rand() % 7;
	return create_piece(random_piece);
}

static void dequeue_next_player_piece() {
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
	last_drop_time = SDL_GetTicks();
	game.start_time = SDL_GetTicks();
	game.current_state = GAME_STATE_PLAYING;
	dequeue_next_player_piece();
}

void prepare_game() {
	game.main_label[0] = '\0';
	for (int i = 0; i < 6; i++) {
		Piece* new_piece = create_random_piece();
		new_piece->row_pos = 3 * i + 1;
		new_piece->col_pos = 1;
		enqueue(next_pieces, new_piece);
		add_piece_to_grid(queue_grid, new_piece, true, false);
	}
	game.level = 1;
	game.score = 0;
	game.total_lines_cleared = 0;
	game.elapsed_time = 0;
	game.total_paused_time = 0;
	game.countdown = 3;
	game.current_state = GAME_STATE_COUNTDOWN;
}

void start_fourty_lines() {
	game.current_mode = FOURTY_LINES;
	prepare_game();
}

void start_blitz() {
	game.current_mode = BLITZ;
	prepare_game();
}

void start_endless() {
	game.current_mode = ENDLESS;
	prepare_game();
}

void main_menu() {
	game.current_state = GAME_STATE_MENU;
	clear_queue(next_pieces);
	clear_grid(queue_grid);
	clear_grid(game_board);
	destroy_piece(player_piece);
	player_piece = NULL;
}

void game_over() {
	game.current_state = GAME_OVER_MENU;
	flags.clearing_rows = false;
	flags.dropping_pieces = false;
	flags.move_player_down = false;
	flags.move_player_left = false;
	flags.move_player_right = false;
	flags.rotate_player = false;
	flags.drop_player = false;
	snprintf(game.main_label, sizeof(game.main_label), "GAME OVER!");
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
	}, title_font, button_font);

	game_over_menu = create_game_over_menu((ButtonCallback[]) {
		main_menu,
		send_quit
	}, button_font);

	game_board = create_grid(BOARD_WIDTH, BOARD_HEIGHT, true);

	queue_grid = create_grid(6, 19, true);
	queue_grid->show_grid_lines = false;

	next_pieces = create_queue(destroy_piece);

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
	destroy_grid(queue_grid);
	destroy_queue(next_pieces);
	destroy_title_menu(title_menu);
	destroy_game_over_menu(game_over_menu);
	player_piece = NULL;
	game_board = NULL;
	queue_grid = NULL;
	next_pieces = NULL;
	title_menu = NULL;
	game_over_menu = NULL;
}

void process_input(bool* running) {
	SDL_Event event;
	SDL_PollEvent(&event);

	if (event.type == SDL_WINDOWEVENT) {
		if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
			int window_width = event.window.data1; 
			int window_height = event.window.data2;
			scale_factor = MIN((float)window_width / WINDOW_WIDTH, (float)window_height / WINDOW_HEIGHT);
			title_menu->scale_factor = scale_factor;
			game_over_menu->scale_factor = scale_factor;
			TTF_CloseFont(label_font);
			label_font = TTF_OpenFont("PoltBold-V5aZ.otf", (int)(LABEL_DEFAULT_FONT_SIZE * scale_factor));
			TTF_CloseFont(label_font_small);
			label_font_small = TTF_OpenFont("Polt-AABM.otf", (int)(LABEL_DEFAULT_SMALL_FONT_SIZE * scale_factor));
		}	
	}

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

	// Debug, should remove later
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
	bool drop_player = false;

	if (game.current_state == GAME_STATE_COUNTDOWN) {
		if (SDL_GetTicks() - game.label_display_start_time >= COUNTDOWN_DISPLAY_DURATION) {
			snprintf(game.main_label, sizeof(game.main_label), "%d", game.countdown);
			game.label_display_start_time = SDL_GetTicks();
			if (game.countdown <= 0) {
				snprintf(game.main_label, sizeof(game.main_label), "GO!");
				start_game();
			}
			game.countdown--;
		}
		return;
	}

	if (game.current_state == GAME_STATE_PLAYING) {
		if (flags.clearing_rows) {
			// Check for full rows
			game.current_lines_cleared = check_and_clear_full_rows(game_board);
			if (game.current_lines_cleared > 0) {
				flags.dropping_pieces = true;
				game.pause_start_time = SDL_GetTicks();
				game.total_lines_cleared += game.current_lines_cleared;
				game.score += game.current_lines_cleared * 10 * game.current_lines_cleared;
				char* label = get_row_clear_label(game.current_lines_cleared);
				snprintf(game.main_label, sizeof(game.main_label), "%s", label);
				game.label_display_start_time = SDL_GetTicks();
			}
			flags.clearing_rows = false;
			return;
		}
		if (flags.dropping_pieces) {
			if (SDL_GetTicks() - game.pause_start_time >= 700) {
				drop_all_pieces(game_board);
				game.total_paused_time += SDL_GetTicks() - game.pause_start_time;
				last_drop_time = SDL_GetTicks();
				flags.dropping_pieces = false;
				// Check for full rows again
				flags.clearing_rows = true;
			}
			return;
		}
		game.elapsed_time = SDL_GetTicks() - game.start_time - game.total_paused_time;

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
			drop_player = true;
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
		bool piece_added = add_piece_to_grid(game_board, player_piece, lock_piece, drop_player);
		bool time_up = game.current_mode == BLITZ && game.elapsed_time > BLITZ_TIME;
		if (!piece_added || time_up) {
			if (time_up) {
				game.elapsed_time = BLITZ_TIME;
			}
			game_over();
			return;
		}

		if (lock_piece) {
			dequeue_next_player_piece();
			
			// Check for full rows on next iteration
			flags.clearing_rows = true;
		}
	}
}

void render(SDL_Renderer* renderer) {
	debug_renderer = renderer;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	if (game.current_state == GAME_STATE_MENU) {
		draw_title_menu(title_menu, renderer);
	}
	else if (game.current_state == GAME_OVER_MENU) {
		draw_game_over_menu(game_over_menu, renderer);
	}
	
	// Still want to show the game board at end of game
	if (game.current_state != GAME_STATE_MENU) {
		// Board and queue grid
		int cell_width = 32 * scale_factor;
		int board_x = (float)WINDOW_WIDTH / 2 - (float)cell_width * BOARD_WIDTH / 2;
		board_x *= scale_factor;
		int board_y = 50;
		draw_grid(game_board, board_x, board_y, cell_width, true, renderer);
		draw_grid(queue_grid, 50 * scale_factor + game_board->width * cell_width + board_x, board_y, cell_width, true, renderer);

		// Stats
		int stats_board_padding = 10 * scale_factor;
		int stats_x = board_x - stats_board_padding;
		int stats_y = board_y + cell_width * BOARD_HEIGHT;

		int stats_vertical_offset = 15 * scale_factor;

		const char* labels[] = {"SCORE", "LEVEL", "LINES", "TIME"};
		int values[] = { game.score, game.level, game.total_lines_cleared, 0 };

		LabelStyle label_style_small_font = default_label_style_no_font();
		label_style_small_font.font = label_font_small;

		LabelStyle label_style_small_font_left = default_label_style_no_font();
		label_style_small_font_left.font = label_font_small;
		label_style_small_font_left.align_right = false;

		LabelStyle label_style_large_font = default_label_style_no_font();
		label_style_large_font.font = label_font;

		// Easier to draw bottom to top in this case
		for (int i = 3; i >= 0; i--) {
			if (i == 2 && game.current_mode == FOURTY_LINES) {
				SDL_Rect small_label = draw_label(renderer, stats_x, stats_y, "/40", label_style_small_font);
				stats_x -= small_label.w;
			}
			else if (i == 3) {
				int time_ms = game.elapsed_time;
				if (game.current_mode == BLITZ) {
					// In this case we count down from 2 minutes
					time_ms = BLITZ_TIME - time_ms;
					if (time_ms < 10000) {
						label_style_large_font.color = (SDL_Color){ 200, 0, 0, 255 };
						label_style_small_font_left.color = (SDL_Color){ 200, 0, 0, 255 };
					}
				}
				int small_label_w, _;
				TTF_SizeText(label_font_small, ".000", &small_label_w, &_);
				char mins_secs_buffer[100];
				char millis_buffer[100];
				time_formater(mins_secs_buffer, millis_buffer, sizeof(mins_secs_buffer), time_ms);
				draw_label(renderer, stats_x - small_label_w, stats_y, millis_buffer, label_style_small_font_left);
				
				stats_x -= small_label_w;
				SDL_Rect large_label = draw_label(renderer, stats_x, stats_y, mins_secs_buffer, label_style_large_font);
				stats_x = board_x - stats_board_padding;
				stats_y -= large_label.h;
				stats_y -= draw_label(renderer, stats_x, stats_y, labels[i], label_style_small_font).h + stats_vertical_offset;
				label_style_large_font.color = label_style_small_font_left.color = default_label_style_no_font().color;
				continue;
			}
			char label[100];
			snprintf(label, sizeof(label), "%d", values[i]);
			stats_y -= draw_label(renderer, stats_x, stats_y, label, label_style_large_font).h;
			stats_x = board_x - stats_board_padding;
			stats_y -= draw_label(renderer, stats_x, stats_y, labels[i], label_style_small_font).h + stats_vertical_offset;
		}

		// Line clear label
		LabelStyle label_style_clear_label = default_label_style_no_font();
		label_style_clear_label.font = label_font;

		if (game.current_state != GAME_OVER_MENU) {
			int fade_duration = game.current_state == GAME_STATE_PLAYING ? ROW_LABEL_DISPLAY_DURATION : COUNTDOWN_DISPLAY_DURATION;
			label_style_clear_label.color.a = get_fade_alpha(game.label_display_start_time, fade_duration);
		}
		draw_label(renderer, stats_x - stats_board_padding, stats_y - stats_vertical_offset, game.main_label, label_style_clear_label);
		
	}

	SDL_RenderPresent(renderer);
}