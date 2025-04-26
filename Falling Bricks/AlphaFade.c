#include "AlphaFade.h"
#include <SDL.h>

Uint8 get_fade_alpha(Uint32 start_time, Uint32 duration) {
	Uint32 now = SDL_GetTicks();
	if (now >= start_time + duration) return 0;

	float progress = (float)(now - start_time) / duration;
	float alpha = 1.0f - progress;

	// Clamp and scale to 0-255 for SDL alpha
	return (Uint8)(alpha * 255);
}