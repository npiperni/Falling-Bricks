#include "Menu.h"
#include "Button.h"
#include "Constants.h"
#include "ResolutionContext.h"
#include "Grid.h"
#include "Piece.h"
#include "FontContext.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>

static bool create_grid_array(struct TitleMenu* menu) {
	// Floating blocks grids.
	menu->floating_grids = create_dynamic_array(20, destroy_grid);
	menu->grid_positions = create_dynamic_array(20, free);
	if (!menu->floating_grids) {
		fprintf(stderr, "Error: Failed to allocate memory for floating grids\n");
		destroy_dynamic_array(menu->floating_grids);
		destroy_dynamic_array(menu->grid_positions);
		return false;
	}
	return true;
}

static void create_grid_piece(struct TitleMenu* menu) {
	// Create a grid for each floating block
	Piece* piece = create_random_piece();
	Grid* floating_grid = create_grid(piece->width, piece->height, false, false);
	SDL_FPoint* position = malloc(sizeof(SDL_FPoint));

	if (!floating_grid || !position) {
		fprintf(stderr, "Error: Failed to create grid for floating block\n");
		destroy_piece(piece);
		destroy_grid(floating_grid);
		free(position);
		return;
	}
	add_piece_to_grid(floating_grid, piece, true, false);
	add_to_dynamic_array(menu->floating_grids, floating_grid); // Grid now belongs to dynamic array. Do not free it here.
	add_to_dynamic_array(menu->grid_positions, position); // Position now belongs to dynamic array. Do not free it here.
	destroy_piece(piece);

	// Calculate x position for piece. Chose random position with a bias towards edges
	float random = (float)rand() / RAND_MAX;
	if (random <= 0.5f) {
		// Bias towards the left edge
		random = -sqrtf(0.25f - powf(random, 2)) + 0.5f;
	}
	else {
		// Bias towards the right edge
		random = sqrtf(0.25f - powf(random - 1, 2)) + 0.5f;
	}

	position->x = random * (WINDOW_WIDTH - floating_grid->width * CELL_SIZE);

	// Spawn piece at top of actual window height (local_y = (absolute_y - y_offset) / scale where absolute_y is 0 for top). This will get scaled properly during drawing
	position->y = -menu->res_context.y_offset / menu->res_context.scale_factor;
	position->y -= 2 * CELL_SIZE;

	menu->floating_grid_creation_time = SDL_GetTicks();
}

static void draw_piece_grids(struct TitleMenu* menu, SDL_Renderer* renderer) {
	for (int i = 0; i < menu->floating_grids->size; i++) {
		Grid* grid = get_from_dynamic_array(menu->floating_grids, i);
		SDL_FPoint* position = get_from_dynamic_array(menu->grid_positions, i);
		int x = position->x * menu->res_context.scale_factor + menu->res_context.x_offset;
		int y = position->y * menu->res_context.scale_factor + menu->res_context.y_offset;
		int cell_width = CELL_SIZE * menu->res_context.scale_factor;
		draw_grid(grid, x, y, cell_width, 0, renderer);
	}
}

struct TitleMenu* create_title_menu(ButtonCallback on_click[4]) {
	struct TitleMenu* menu = malloc(sizeof(struct TitleMenu));
	if (!menu) {
		fprintf(stderr, "Error: Failed to allocate memory for TitleMenu\n");
		return NULL;
	}
	
	if (!create_grid_array(menu)) {
		free(menu);
		return NULL;
	}

	FontContext* font_context = get_font_context();
	TTF_Font* title_font = font_context->title_font;
	TTF_Font* button_font = font_context->button_font;

	SDL_Color button_color = { 50, 50, 50, SDL_ALPHA_OPAQUE };
	int button_width = 200;
	int button_height = 50;
	int button_x = (WINDOW_WIDTH - button_width) / 2;
	int button_y = WINDOW_HEIGHT / 2 - 200;
	menu->buttons[0] = create_button(button_x, button_y + 100, button_width, button_height, button_color, on_click[0], "40 Lines", button_font);
	menu->buttons[1] = create_button(button_x, button_y + 200, button_width, button_height, button_color, on_click[1], "Blitz", button_font);
	menu->buttons[2] = create_button(button_x, button_y + 300, button_width, button_height, button_color, on_click[2], "Endless", button_font);
	menu->buttons[3] = create_button(button_x, button_y + 400, button_width, button_height, button_color, on_click[3], "Quit", button_font);
	menu->res_context = get_resolution_context(WINDOW_WIDTH, WINDOW_HEIGHT);
	menu->floating_grid_creation_time = SDL_GetTicks();

	SDL_Surface* title_surface = TTF_RenderText_Solid(title_font, "Falling Bricks", (SDL_Color) { 200, 175, 0, SDL_ALPHA_OPAQUE });
	menu->title_texture = SDL_CreateTextureFromSurface(SDL_GetRenderer(SDL_GetWindowFromID(1)), title_surface);
	SDL_FreeSurface(title_surface);
	return menu;
}

void draw_title_menu(struct TitleMenu* menu, SDL_Renderer* renderer) {
	draw_piece_grids(menu, renderer);
	SDL_Rect title_rect = { (WINDOW_WIDTH - 750) / 2, 50, 750, 125 };
	ResolutionContext context = menu->res_context;
	title_rect.x = title_rect.x * context.scale_factor + context.x_offset;
	title_rect.y = title_rect.y * context.scale_factor + context.y_offset;
	title_rect.w *= context.scale_factor;
	title_rect.h *= context.scale_factor;
	SDL_RenderCopy(renderer, menu->title_texture, NULL, &title_rect);

	for (int i = 0; i < 4; i++) {
		menu->buttons[i]->res_context = menu->res_context;
		draw_button(menu->buttons[i], renderer);
	}
}

void update_grid_positions(struct TitleMenu* menu, float delta_time) {
	int _, window_height;
	SDL_GetWindowSize(SDL_GetWindowFromID(1), &_, &window_height);
	for (int i = 0; i < menu->floating_grids->size; i++) {
		SDL_FPoint* position = get_from_dynamic_array(menu->grid_positions, i);
		position->y += delta_time * 50;
		float pos_y = position->y * menu->res_context.scale_factor + menu->res_context.y_offset; // The absolute y position in the window
		if (pos_y > window_height) {
			remove_from_dynamic_array(menu->grid_positions, position);
			free(position);

			Grid* grid = get_from_dynamic_array(menu->floating_grids, i);
			remove_from_dynamic_array(menu->floating_grids, grid);
			destroy_grid(grid);
			i--;
		}
	}
	if (SDL_GetTicks() - menu->floating_grid_creation_time > BLOCK_INTERVAL) {
		create_grid_piece(menu);
		menu->floating_grid_creation_time = SDL_GetTicks();
	}
}

void handle_title_menu_events(struct TitleMenu* menu, SDL_Event event) {
	for (int i = 0; i < 4; i++) {
		handle_button_event(menu->buttons[i], event);
	}
}

void destroy_title_menu(struct TitleMenu* menu) {
	if (!menu) return;
	SDL_DestroyTexture(menu->title_texture);
	for (int i = 0; i < 4; i++) {
		destroy_button(menu->buttons[i]);
	}
	destroy_dynamic_array(menu->floating_grids);
	destroy_dynamic_array(menu->grid_positions);
	free(menu);
}

struct GameOverMenu* create_game_over_menu(ButtonCallback on_click[2]) {
	struct GameOverMenu* menu = malloc(sizeof(struct GameOverMenu));
	if (!menu) {
		fprintf(stderr, "Error: Failed to allocate memory for GameOverMenu\n");
		return NULL;
	}

	FontContext* font_context = get_font_context();
	TTF_Font* button_font = font_context->button_font;

	SDL_Color button_color = { 50, 50, 50, SDL_ALPHA_OPAQUE };
	int button_width = 200;
	int button_height = 50;
	int button_x = 100;
	int button_y = (WINDOW_HEIGHT - button_height) / 2 - 100;
	menu->buttons[0] = create_button(button_x, button_y + 100, button_width, button_height, button_color, on_click[0], "Menu", button_font);
	menu->buttons[1] = create_button(button_x, button_y + 200, button_width, button_height, button_color, on_click[1], "Quit", button_font);
	menu->res_context = get_resolution_context(WINDOW_WIDTH, WINDOW_HEIGHT);
	return menu;
}

void draw_game_over_menu(struct GameOverMenu* menu, SDL_Renderer* renderer) {
	for (int i = 0; i < 2; i++) {
		menu->buttons[i]->res_context = menu->res_context;
		draw_button(menu->buttons[i], renderer);
	}
}

void handle_game_over_menu_events(struct GameOverMenu* menu, SDL_Event event) {
	for (int i = 0; i < 2; i++) {
		handle_button_event(menu->buttons[i], event);
	}
}

void destroy_game_over_menu(struct GameOverMenu* menu) {
	if (!menu) return;
	for (int i = 0; i < 2; i++) {
		destroy_button(menu->buttons[i]);
	}
	free(menu);
}