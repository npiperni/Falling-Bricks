#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Constants.h"
#include "Paths.h"
#include "ToggleIcon.h"
#include "Grid.h"
#include "Piece.h"
#include "Queue.h"
#include "DynamicArray.h"
#include "Menu.h"
#include "Label.h"
#include "AlphaFade.h"
#include "LevelBar.h"
#include "ResolutionContext.h"
#include "AudioContext.h"
#include "FontContext.h"
#include "Game.h"

int last_frame_time = 0;

ResolutionContext resolution_context;

Piece* player_piece = NULL;
Grid* game_board = NULL;

Grid* queue_grid = NULL;
Queue* next_pieces = NULL;

//SDL_Renderer* debug_renderer = NULL;

struct TitleMenu* title_menu = NULL;
struct GameOverMenu* game_over_menu = NULL;
ToggleIcon* music_icon = NULL;
ToggleIcon* sound_icon = NULL;

typedef enum {
	GAME_STATE_MENU,
	GAME_OVER_MENU,
	GAME_STATE_PLAYING,
	GAME_STATE_COUNTDOWN,
	GAME_STATE_PAUSED
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
	int lines_cleared_this_level;
	Uint32 last_player_drop_time;
	Uint32 start_time;
	Uint32 row_clear_start_time;
	Uint32 total_row_clear_time;
	Uint32 elapsed_time;
	int current_lines_cleared;
	char main_label[20];
	Uint32 label_display_start_time;
	Uint32 level_up_label_display_start_time;
	Uint32 combo_label_display_start_time;
	int countdown;
	Uint32 drop_delay;
	int required_lines_level_up;
	Uint32 game_pause_start_time;
	Uint32 total_pause_time;
} game = { 0 };

struct Flags {
	bool move_player_down;
	bool move_player_left;
	bool move_player_right;
	bool rotate_player;
	bool clockwise_rotation;
	bool drop_player;
	bool check_full_rows;
	bool dropping_pieces;
	bool combo;
} flags = { 0 };

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

static void update_drop_speed() {
	game.drop_delay = MAX(100, BASE_DROP_DELAY * pow(0.93, (game.level - 1)));
}

static void update_level() {
	if (game.lines_cleared_this_level >= game.required_lines_level_up) {
		game.lines_cleared_this_level -= game.required_lines_level_up; // Reset lines cleared this level and carry over the rest to next level
		game.required_lines_level_up = BASE_LINES_PER_LEVEL * pow(1.1, game.level++); // Increase the number of lines needed for next level, then increment level
		game.level_up_label_display_start_time = SDL_GetTicks();
		update_drop_speed();
	}
}

static void calculate_score() {
	game.score += game.current_lines_cleared * BASE_LINE_SCORE * game.current_lines_cleared * game.level * (flags.combo ? COMBO_MULTIPLIER : 1);
}

//static void screenshot_debug() {
//	char buffer[100];
//	sprintf_s(buffer, sizeof(buffer), "%d.bmp", (int)SDL_GetTicks());
//	SDL_Surface* sshot = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
//	SDL_RenderReadPixels(debug_renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
//	SDL_SaveBMP(sshot, buffer);
//	SDL_FreeSurface(sshot);
//}

void start_game() {
	game.last_player_drop_time = game.start_time = SDL_GetTicks();
	game.current_state = GAME_STATE_PLAYING;
	dequeue_next_player_piece();
	play_random_music();
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
	game.lines_cleared_this_level = 0;
	game.elapsed_time = 0;
	game.total_row_clear_time = 0;
	game.countdown = 3;
	game.total_pause_time = 0;
	game.drop_delay = BASE_DROP_DELAY;
	game.required_lines_level_up = BASE_LINES_PER_LEVEL;
	game.level_up_label_display_start_time = SDL_GetTicks() - LEVEL_UP_LABEL_DISPLAY_DURATION; // Prevents showing the label at start of game
	game.combo_label_display_start_time = SDL_GetTicks() - COMBO_LABEL_DISPLAY_DURATION; // Prevents showing the label at start of game
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
	flags.check_full_rows = false;
	flags.dropping_pieces = false;
	flags.move_player_down = false;
	flags.move_player_left = false;
	flags.move_player_right = false;
	flags.rotate_player = false;
	flags.drop_player = false;
	snprintf(game.main_label, sizeof(game.main_label), "GAME OVER!");
	Mix_HaltMusic();
	Mix_HaltChannel(-1); // Stop all channels so we can play the game over sound if anything else is playing
	play_sound(GAME_OVER_SFX);
}

void send_quit() {
	SDL_PushEvent(&(SDL_Event) { .type = SDL_QUIT });
}

void play_next_music() {
	if (game.current_state == GAME_STATE_PLAYING || game.current_state == GAME_STATE_PAUSED) {
		play_random_music();
	}
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
	music_icon = create_toggle_icon((SDL_Rect) { WINDOW_WIDTH - 35, 5, 30, 30 }, MUSIC_ICON_ON, MUSIC_ICON_OFF);
	sound_icon = create_toggle_icon((SDL_Rect) { WINDOW_WIDTH - 70, 5, 30, 30 },SOUND_ICON_ON, SOUND_ICON_OFF);
	if (!music_icon || !sound_icon) {
		fprintf(stderr, "Error: Failed to load icons\n");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "MISSING ICONS", "Failed to load required icons.", 0);
		return false;
	}

	if (!create_font_context()) {
		fprintf(stderr, "Error: Failed to create font context\n");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "MISSING FONTS", "Failed to load required fonts.", 0);
		return false;
	}

	if (!create_audio_context()) {
		fprintf(stderr, "Error: Failed to create audio context\n");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "MISSING AUDIO", "Failed to load required audio files.", 0);
		return false;
	}
	Mix_HookMusicFinished(play_next_music);

	resolution_context = get_resolution_context(WINDOW_WIDTH, WINDOW_HEIGHT);

	title_menu = create_title_menu((ButtonCallback[]) {
		start_fourty_lines,
		start_blitz,
		start_endless,
		send_quit
	});

	game_over_menu = create_game_over_menu((ButtonCallback[]) {
		main_menu,
		send_quit
	});

	game_board = create_grid(BOARD_WIDTH, BOARD_HEIGHT, true, true);

	queue_grid = create_grid(6, 19, true,false);
	queue_grid->show_grid_lines = false;

	next_pieces = create_queue(destroy_piece);

	if (!game_board || !queue_grid || !title_menu || !game_over_menu)
	{
		fprintf(stderr, "Fatal Error during game setup\n"); 
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
	destroy_audio_context();
	destroy_font_context();
	destroy_toggle_icon(music_icon);
	destroy_toggle_icon(sound_icon);
	player_piece = NULL;
	game_board = NULL;
	queue_grid = NULL;
	next_pieces = NULL;
	title_menu = NULL;
	game_over_menu = NULL;
	music_icon = NULL;
	sound_icon = NULL;
}

void process_input(bool* running) {
	SDL_Event event;
	SDL_PollEvent(&event);

	if (event.type == SDL_WINDOWEVENT) {
		if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
			int window_width = event.window.data1; 
			int window_height = event.window.data2;
			resolution_context = get_resolution_context(window_width, window_height);
			title_menu->res_context = resolution_context;
			game_over_menu->res_context = resolution_context;
			music_icon->res_context = resolution_context;
			sound_icon->res_context = resolution_context;
			adjust_label_font_size(resolution_context.scale_factor);
		}	
	}

	if (event.type == SDL_QUIT) {
		*running = false;
		return;
	}

	if (game.current_state == GAME_STATE_MENU) {
		handle_title_menu_events(title_menu, event);
	}
	if (game.current_state == GAME_OVER_MENU) {
		handle_game_over_menu_events(game_over_menu, event);
	}

	AudioContext* audio_context = get_audio_context();

	if (event.type == SDL_KEYDOWN) {
		int key = event.key.keysym.sym;
		if (key == SDLK_ESCAPE) {
			*running = false;
		}
		else if (key == SDLK_m) {
			if (audio_context->music_paused) {
				music_icon->is_toggled = true;
				unpause_music();
			}
			else {
				music_icon->is_toggled = false;
				pause_music();
			}
		}
		else if (key == SDLK_n) {
			if (audio_context->sound_enabled) {
				sound_icon->is_toggled = false;
				disable_sound();
			}
			else {
				sound_icon->is_toggled = true;
				enable_sound();
			}
		}

		if (key == SDLK_p) {
			if (game.current_state == GAME_STATE_PLAYING) {
				game.game_pause_start_time = SDL_GetTicks();
				game.current_state = GAME_STATE_PAUSED;
			}
			else if (game.current_state == GAME_STATE_PAUSED) {
				Uint32 pause_duration = SDL_GetTicks() - game.game_pause_start_time;
				game.total_pause_time += pause_duration;
				game.last_player_drop_time += pause_duration;
				game.current_state = GAME_STATE_PLAYING;
			}
		}

		if (game.current_state == GAME_STATE_PLAYING) {
			if (key == SDLK_UP || key == SDLK_x) {
				flags.rotate_player = true;
				flags.clockwise_rotation = true;
			}
			else if (key == SDLK_z) {
				flags.rotate_player = true;
				flags.clockwise_rotation = false;
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
	
}

void update() {
	Uint32 time_now = SDL_GetTicks();

	int time_to_wait = FRAME_TARGET_TIME - (time_now - last_frame_time);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	float delta_time = (time_now - last_frame_time) / 1000.0f;
	last_frame_time = time_now;

	if (game.current_state == GAME_STATE_MENU) {
		update_grid_positions(title_menu, delta_time);
		return;
	}

	if (game.current_state == GAME_STATE_PAUSED) {
		return;
	}

	bool lock_piece = false;
	bool drop_player = false;

	if (game.current_state == GAME_STATE_COUNTDOWN) {
		if (time_now - game.label_display_start_time >= COUNTDOWN_DISPLAY_DURATION) {
			snprintf(game.main_label, sizeof(game.main_label), "%d", game.countdown);
			game.label_display_start_time = time_now;
			if (game.countdown <= 0) {
				snprintf(game.main_label, sizeof(game.main_label), "GO!");
				start_game();
			}
			game.countdown--;
		}
		return;
	}

	AudioContext* audio_context = get_audio_context();

	if (game.current_state == GAME_STATE_PLAYING) {

		update_level();
		if (flags.check_full_rows) {
			// Check for full rows
			game.current_lines_cleared = check_and_mark_full_rows(game_board);
			if (game.current_lines_cleared > 0) {
				flags.dropping_pieces = true;
				game.row_clear_start_time = time_now;
				game.total_lines_cleared += game.current_lines_cleared;
				game.lines_cleared_this_level += game.current_lines_cleared;
				calculate_score();
				char* label = get_row_clear_label(game.current_lines_cleared);
				snprintf(game.main_label, sizeof(game.main_label), "%s", label);
				game.label_display_start_time = time_now;
				game.combo_label_display_start_time = flags.combo ? time_now : 0;
				play_sound(CLEAR_SFX);
			}
			flags.combo = false;
			flags.check_full_rows = false;
			return;
		}
		if (flags.dropping_pieces) {
			if (time_now - game.row_clear_start_time >= ROW_CLEAR_TIME) {
				clear_full_rows(game_board);
				drop_all_pieces(game_board);
				game.total_row_clear_time += time_now - game.row_clear_start_time;
				game.last_player_drop_time = time_now;
				flags.dropping_pieces = false;
				// Check for full rows again
				flags.check_full_rows = true;
				flags.combo = true;
			}
			return;
		}
		game.elapsed_time = time_now - game.start_time - game.total_row_clear_time - game.total_pause_time;
		bool time_up = game.current_mode == BLITZ && game.elapsed_time > BLITZ_TIME;
		bool reached_line_limit = game.current_mode == FOURTY_LINES && game.total_lines_cleared >= 40;
		if ( time_up || reached_line_limit) {
			if (time_up) {
				game.elapsed_time = BLITZ_TIME;
			}
			game_over();
			return;
		}
		if (player_piece == NULL) {
			dequeue_next_player_piece();
		}

		if (time_now - game.last_player_drop_time >= game.drop_delay) {
			game.last_player_drop_time = time_now;
			flags.move_player_down = true;
		}

		if (flags.move_player_left) {
			if (move_player_left()) {
				play_sound(MOVE_SFX);
			}
			flags.move_player_left = false;
		}
		if (flags.move_player_right) {
			if (move_player_right()) {
				play_sound(MOVE_SFX);
			}
			flags.move_player_right = false;
		}
		if (flags.rotate_player) {
			Piece* rotated_piece = try_rotate_piece(game_board, player_piece, flags.clockwise_rotation);
			if (rotated_piece) {
				destroy_piece(player_piece);
				// Update to rotated piece and new position after rotation
				player_piece = rotated_piece;
				play_sound(MOVE_SFX);
			}
			flags.rotate_player = false;
		}
		if (flags.move_player_down) {
			lock_piece = !move_player_down();
			flags.move_player_down = false;
		}
		if (flags.drop_player) {
			drop_player = true;
			lock_piece = true;
			flags.drop_player = false;
			game.last_player_drop_time = time_now;
		}

		clear_unlocked_cells(game_board);
		bool piece_added = add_piece_to_grid(game_board, player_piece, lock_piece, drop_player);
		if (!piece_added) {
			// If the piece can't be added, it means it has reached the top of the board
			mark_x_cells(game_board, player_piece);
			game_over();
			return;
		}

		if (lock_piece) {
			destroy_piece(player_piece);
			player_piece = NULL;
			// Check for full rows on next iteration
			flags.check_full_rows = true;
			play_sound(LOCK_SFX);
		}
	}
}

void render(SDL_Renderer* renderer) {
	//debug_renderer = renderer;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	if (game.current_state == GAME_STATE_MENU) {
		draw_title_menu(title_menu, renderer);
	}
	else if (game.current_state == GAME_OVER_MENU) {
		draw_game_over_menu(game_over_menu, renderer);
	}

	// Draw icons
	draw_toggle_icon(music_icon, renderer);
	draw_toggle_icon(sound_icon, renderer);

	FontContext* font_context = get_font_context();

	if (game.current_state == GAME_STATE_PAUSED) {
		// draw pause text in middle of screen
		LabelStyle label_style = default_label_style_no_font();
		label_style.font = font_context->label_font;
		label_style.align_right = false;
		label_style.align_bottom = false;
		int label_width, label_height;
		TTF_SizeText(label_style.font, "GAME PAUSED", &label_width, &label_height);

		// Align center
		int x_pos = WINDOW_WIDTH / 2;
		int y_pos = WINDOW_HEIGHT / 2;
		x_pos = x_pos * resolution_context.scale_factor + resolution_context.x_offset - label_width / 2;
		y_pos = y_pos * resolution_context.scale_factor + resolution_context.y_offset - label_height / 2;
		draw_label(renderer, x_pos, y_pos, "GAME PAUSED", label_style);
	}
	
	// Still want to show the game board at end of game and during countdown
	if (game.current_state != GAME_STATE_MENU && game.current_state != GAME_STATE_PAUSED) {

		float scale_factor = resolution_context.scale_factor;
		int border_width = 4 * scale_factor;

		// Board
		int board_x = ((float)WINDOW_WIDTH / 2 - (float)CELL_SIZE * BOARD_WIDTH / 2  ) * scale_factor + resolution_context.x_offset;
		int board_y = ((float)WINDOW_HEIGHT / 2 - (float)CELL_SIZE * BOARD_HEIGHT / 2) * scale_factor + resolution_context.y_offset;
		int cell_width = CELL_SIZE * scale_factor;
		draw_grid(game_board, board_x, board_y, cell_width, border_width, renderer);

		// Level bar
		int x_pos = 5 * scale_factor + game_board->width * cell_width + board_x;
		int y_pos = board_y;
		int w = 20 * scale_factor;
		int h = cell_width * BOARD_HEIGHT;
		draw_level_bar(renderer, x_pos, y_pos, w, h, border_width, game.lines_cleared_this_level, game.required_lines_level_up);

		x_pos += w + 5 * scale_factor;
		// Queue grid
		draw_grid(queue_grid, x_pos, board_y, cell_width, border_width, renderer);

		// Stats
		int stats_board_padding = 10 * scale_factor;
		int stats_x = board_x - stats_board_padding;
		int stats_y = board_y + cell_width * BOARD_HEIGHT;

		int stats_vertical_offset = 15 * scale_factor;

		const char* labels[] = {"SCORE", "LEVEL", "LINES", "TIME"};
		int values[] = { game.score, game.level, game.total_lines_cleared, 0 };

		LabelStyle label_style_small_font = default_label_style_no_font();
		label_style_small_font.font = font_context->label_font_small;

		LabelStyle label_style_small_font_left = default_label_style_no_font();
		label_style_small_font_left.font = font_context->label_font_small;
		label_style_small_font_left.align_right = false;

		LabelStyle label_style_large_font = default_label_style_no_font();
		label_style_large_font.font = font_context->label_font;

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
				TTF_SizeText(font_context->label_font_small, ".000", &small_label_w, &_);
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

		// Line clear and countdown label (used for both)
		LabelStyle label_style = default_label_style_no_font();
		label_style.font = font_context->label_font;

		if (game.current_state != GAME_OVER_MENU) {
			int fade_duration = game.current_state == GAME_STATE_PLAYING ? ROW_LABEL_DISPLAY_DURATION : COUNTDOWN_DISPLAY_DURATION;
			label_style.color.a = get_fade_alpha(game.label_display_start_time, fade_duration);
		}
		stats_y -= draw_label(renderer, stats_x - stats_board_padding, stats_y - stats_vertical_offset, game.main_label, label_style).h + stats_vertical_offset;
		
		// Combo label
		label_style.color = (SDL_Color){ 0, 255, 0, 255 };
		label_style.color.a = get_fade_alpha(game.combo_label_display_start_time, COMBO_LABEL_DISPLAY_DURATION);
		stats_y -= draw_label(renderer, stats_x - stats_board_padding, stats_y - stats_vertical_offset, "GRAVITY COMBO!", label_style).h + stats_vertical_offset;

		// Level up label
		label_style.color = (SDL_Color){ 233, 200, 0, 255 };
		label_style.color.a = get_fade_alpha(game.level_up_label_display_start_time, LEVEL_UP_LABEL_DISPLAY_DURATION);
		draw_label(renderer, stats_x - stats_board_padding, stats_y - stats_vertical_offset, "LEVEL UP!", label_style);
	}

	SDL_RenderPresent(renderer);
}