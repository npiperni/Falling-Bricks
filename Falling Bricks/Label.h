#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

typedef struct {
    SDL_Color color;
    bool align_right;
    bool align_bottom;
    TTF_Font* font;
} LabelStyle;


/// <summary>
/// Returns a rectangle containing the label drawn at the specified position.
/// </summary>
/// <param name="renderer"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="label"></param>
/// <param name="label_style"></param>
/// <returns></returns>
SDL_Rect draw_label(SDL_Renderer* renderer, int x, int y, const char* label, LabelStyle label_style);

/// <summary>
/// Returns a default label style with no font. Caller must set the font.
/// </summary>
LabelStyle default_label_style_no_font();

void time_formater(char* mins_secs_buffer, char* millis_buffer, size_t size_of_buffers, Uint32 time_in_ms);

const char* get_row_clear_label(int rows);
