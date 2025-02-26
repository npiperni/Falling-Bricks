#include <stdio.h>
#include <SDL.h>
#include "Constants.h"

#include "Grid.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int game_is_running = FALSE;

int last_frame_time = 0;

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

int init_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error intitializing SDL.\n");
		return FALSE;
	}
	window = SDL_CreateWindow(
		"My SDL Title",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		//SDL_WINDOW_BORDERLESS
		SDL_WINDOW_RESIZABLE
	);
	if (!window) {
		fprintf(stderr, "Error creating SDL window.\n");
		return FALSE;
	}
	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		fprintf(stderr, "Error creating SDL renderer.\n");
		return FALSE;
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	return TRUE;
}

void setup() {
	ball.x = 20;
	ball.y = 20;
	ball.width = 15;
	ball.height = 15;

	player.row = 0;
	player.col = 0;
}

void process_input() {
	SDL_Event event;
	SDL_PollEvent(&event);

	int key = 0;
	switch (event.type) {
	case SDL_QUIT:
		game_is_running = FALSE;
		break;
	case SDL_KEYDOWN:
		key = event.key.keysym.sym;
		if (key == SDLK_ESCAPE) {
			game_is_running = FALSE;
		} 
		else if (key == SDLK_UP) {
			ball.y -= 50;
			player.row--;
		} 
		else if (key == SDLK_DOWN) {
			ball.y += 50;
			player.row++;
		} 
		else if (key == SDLK_LEFT) {
			ball.x -= 50;
			player.col--;
		} 
		else if (key == SDLK_LEFT) {
			ball.x -= 50;
			player.col--;
		}
		else if (key == SDLK_RIGHT) {
			ball.x += 50;
			player.col++;
		}
		break;
	case SDL_MOUSEMOTION:
		//printf("Mouse moved.\n");
		printf("%d %d\n", event.motion.x, event.motion.y);
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
	/*if (delta_time < 5)
	{
		return;
	}*/

	last_frame_time = SDL_GetTicks();

	ball.x += 70 * delta_time;
	ball.y += 50 * delta_time;
	//printf("%d\n", SDL_GetTicks());

	int current_width, current_height;
	SDL_GetWindowSize(window, &current_width, &current_height);
	scale_factor = MIN((float) current_width / WINDOW_WIDTH, (float) current_height / WINDOW_HEIGHT);
	//printf("%f\n", scale_factor);
	int displayIndex = SDL_GetWindowDisplayIndex(window);
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(displayIndex, &display_mode);
	//printf("%d %d\n", display_mode.w, display_mode.h);
}

void render() {
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

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLine(renderer, 5, 5, 160, 130);

	SDL_Rect my_rect = { 100 * scale_factor, 100 * scale_factor, 100 * scale_factor, 100 * scale_factor };
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &my_rect);

	SDL_Surface* screen = SDL_GetWindowSurface(window);

	//// Draw grid
	Grid* grid = create_grid(10, 20, true);
	grid->cells[player.row][player.col].color = (SDL_Color){ 0, 177, 0, 100 };
	grid->cells[player.row][player.col].outline = true;
	draw_grid(grid, renderer);
	destroy_grid(grid);

	SDL_Rect rect1 = { 400, 400, 100, 100 };
	SDL_Rect rect2 = { 400, 400, 100, 100 };

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect1);

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &rect2);

	SDL_RenderPresent(renderer);
}

void destroy_window() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char* args[]) {
	
	game_is_running = init_window();
	
	setup();

	while (game_is_running) {
		process_input();
		update();
		render();
	}

	destroy_window();

	return 0;
}