#pragma once
#include <SDL_mixer.h>
#include <stdbool.h>

#define NUM_SONGS 5

typedef enum {
	MOVE_SFX,
	LOCK_SFX,
	CLEAR_SFX,
	GAME_OVER_SFX
} Sound;

typedef struct {
	Mix_Music* music[NUM_SONGS];
	Mix_Chunk* move_sound;
	Mix_Chunk* lock_sound;
	Mix_Chunk* clear_sound;
	Mix_Chunk* game_over;
	bool music_paused;
	bool sound_enabled;
} AudioContext;

bool create_audio_context();

AudioContext* get_audio_context();

void play_random_music();

void pause_music();

void unpause_music();

void enable_sound();

void disable_sound();

void play_sound(Sound sound);

void destroy_audio_context();