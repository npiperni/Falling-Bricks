#include "FontContext.h"
#include "Constants.h"
#include "Paths.h"
#include "stdio.h"
#include <SDL_ttf.h>
#include <stdbool.h>

static FontContext* font_context = NULL; // Singleton instance

bool create_font_context() {
	if (font_context) {
		fprintf(stderr, "FontContext already created.\n");
		return false;
	}
	if (TTF_Init() == -1) {
		fprintf(stderr, "Error initializing TTF: %s\n", TTF_GetError());
		return false;
	}
	font_context = (FontContext*)malloc(sizeof(FontContext));
	if (!font_context) {
		fprintf(stderr, "Error allocating memory for FontContext.\n");
		return false;
	}
	font_context->button_font = TTF_OpenFont(BUTTON_FONT, 72);
	font_context->title_font = TTF_OpenFont(TITLE_FONT, 128);
	font_context->label_font = TTF_OpenFont(LABEL_FONT, LABEL_DEFAULT_FONT_SIZE);
	font_context->label_font_small = TTF_OpenFont(LABEL_FONT_SMALL, LABEL_DEFAULT_SMALL_FONT_SIZE);
	if (!font_context->button_font || !font_context->title_font || !font_context->label_font || !font_context->label_font_small) {
		fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
		destroy_font_context();
		return false;
	}
	return true;
}

FontContext* get_font_context() {
	return font_context;
}

void adjust_label_font_size(float scale_factor) {
	if (font_context) {
		TTF_CloseFont(font_context->label_font);
		font_context->label_font = TTF_OpenFont(LABEL_FONT, (int)(LABEL_DEFAULT_FONT_SIZE * scale_factor));
		TTF_CloseFont(font_context->label_font_small);
		font_context->label_font_small = TTF_OpenFont(LABEL_FONT_SMALL, (int)(LABEL_DEFAULT_SMALL_FONT_SIZE * scale_factor));
	}
}

void destroy_font_context() {
	if (font_context) {
		TTF_CloseFont(font_context->button_font);
		TTF_CloseFont(font_context->title_font);
		TTF_CloseFont(font_context->label_font);
		TTF_CloseFont(font_context->label_font_small);
		free(font_context);
		TTF_Quit();
		font_context = NULL;
	}
}