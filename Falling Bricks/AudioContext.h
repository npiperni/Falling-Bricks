#pragma once
#include <SDL_mixer.h>

#define NUM_SONGS 5

typedef struct {
	Mix_Music* music[NUM_SONGS];
	Mix_Chunk* move_sound;
	Mix_Chunk* lock_sound;
	Mix_Chunk* clear_sound;
	Mix_Chunk* game_over;
} AudioContext;

AudioContext* create_audio_context();

void destroy_audio_context(AudioContext* audio_context);