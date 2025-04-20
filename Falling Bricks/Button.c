#include "Button.h"
#include <SDL.h>
#include <stdio.h>

static SDL_Rect get_scaled_rect(Button* button) {
	SDL_Rect scaled_rect = {
		button->rect.x * button->scale_factor,
		button->rect.y * button->scale_factor,
		button->rect.w * button->scale_factor,
		button->rect.h * button->scale_factor
	};
	return scaled_rect;
}

static void handle_hover(Button* button) {
	int mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);
	SDL_Rect scaled_rect = get_scaled_rect(button);
	if (mouse_x >= scaled_rect.x && mouse_x <= scaled_rect.x + scaled_rect.w &&
		mouse_y >= scaled_rect.y && mouse_y <= scaled_rect.y + scaled_rect.h) {
		button->hovered = true;
	}
	else {
		button->hovered = false;
	}
}

Button* create_button(int x, int y, int width, int height, SDL_Color color, ButtonCallback on_click, char* label, TTF_Font* font) {
	Button* button = malloc(sizeof(Button));
	if (!button) {
		fprintf(stderr, "Error: Failed to allocate memory for Button\n");
		return NULL;
	}
	button->rect.x = x;
	button->rect.y = y;
	button->rect.w = width;
	button->rect.h = height;
	button->color = color;
	button->hovered = false;
	button->on_click = on_click;
	button->scale_factor = 1.0f;

	button->label = label;
	SDL_Surface* text_surface = TTF_RenderText_Solid(font, label, (SDL_Color) { 255, 255, 255, SDL_ALPHA_OPAQUE });
	button->texture = SDL_CreateTextureFromSurface(SDL_GetRenderer(SDL_GetWindowFromID(1)), text_surface);
	SDL_FreeSurface(text_surface);

	return button;
}

void destroy_button(Button* button) {
	free(button);
}

void draw_button(Button* button, SDL_Renderer* renderer) {
	SDL_Rect scaled_rect = get_scaled_rect(button);

	SDL_SetRenderDrawColor(renderer, button->color.r, button->color.g, button->color.b, button->color.a);
	SDL_RenderFillRect(renderer, &scaled_rect);
	SDL_Rect text_rect = { scaled_rect.x + scaled_rect.w / 8, scaled_rect.y + scaled_rect.h / 6, scaled_rect.w * 3 / 4, scaled_rect.h * 3 / 4};
	SDL_RenderCopy(renderer, button->texture, NULL, &text_rect);
	if (button->hovered) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &scaled_rect);
	}
}

void handle_button_event(Button* button, SDL_Event event) {
	handle_hover(button);

	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && button->hovered) {
		button->on_click();
	}
}