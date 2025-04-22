#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

/// <summary>
/// Returns a rectangle containing the label drawn at the specified position.
/// </summary>
/// <param name="renderer"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="label"></param>
/// <param name="font"></param>
/// <param name="alignLeft"></param>
/// <returns></returns>
SDL_Rect draw_label(SDL_Renderer* renderer, int x, int y, const char* label, TTF_Font* font, bool alignLeft);
