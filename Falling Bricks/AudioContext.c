#include "AudioContext.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL_mixer.h>

static AudioContext* audio_context = NULL; // Singleton instance

bool create_audio_context() {
	if (audio_context) {
		fprintf(stderr, "AudioContext already created.\n");
		return false;
	}
	audio_context = malloc(sizeof(AudioContext));
	if (!audio_context) {
		fprintf(stderr, "Error: Failed to allocate memory for AudioContext\n");
		return false;
	}

	if (!Mix_Init(MIX_INIT_MP3)) {
		fprintf(stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
		free(audio_context);
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		fprintf(stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
		free(audio_context);
		return false;
	}

	audio_context->music[0] = Mix_LoadMUS("assets/audio/music/1.mp3");
	audio_context->music[1] = Mix_LoadMUS("assets/audio/music/2.mp3");
	audio_context->music[2] = Mix_LoadMUS("assets/audio/music/3.mp3");
	audio_context->music[3] = Mix_LoadMUS("assets/audio/music/4.mp3");
	audio_context->music[4] = Mix_LoadMUS("assets/audio/music/5.mp3");
	audio_context->move_sound = Mix_LoadWAV("assets/audio/sounds/click.wav");
	audio_context->lock_sound = Mix_LoadWAV("assets/audio/sounds/lock.wav");
	audio_context->clear_sound = Mix_LoadWAV("assets/audio/sounds/clear.wav");
	audio_context->game_over = Mix_LoadWAV("assets/audio/sounds/game_over.wav");
	if (!audio_context->move_sound || !audio_context->lock_sound || !audio_context->clear_sound || !audio_context->game_over) {
		fprintf(stderr, "Error loading sound file: %s\n", Mix_GetError());
		destroy_audio_context(audio_context);
		return false;
	}

	for (int i = 0; i < NUM_SONGS; i++) {
		if (!audio_context->music[i]) {
			fprintf(stderr, "Error loading audio file %d: %s\n", i, Mix_GetError());
			destroy_audio_context(audio_context);
			return false;
		}
	}

	return true;
}

AudioContext* get_audio_context() {
	return audio_context;
}

void play_random_music() {
	if (audio_context) {
		int random_index = rand() % NUM_SONGS;
		if (Mix_PlayMusic(audio_context->music[random_index], 0) == -1) {
			fprintf(stderr, "Error playing music: %s\n", Mix_GetError());
		}
	}
}

void destroy_audio_context() {
	if (audio_context) {
		for (int i = 0; i < NUM_SONGS; i++) {
			if (audio_context->music[i]) {
				Mix_FreeMusic(audio_context->music[i]);
			}
		}
		if (audio_context->move_sound) {
			Mix_FreeChunk(audio_context->move_sound);
		}
		if (audio_context->lock_sound) {
			Mix_FreeChunk(audio_context->lock_sound);
		}
		free(audio_context);
		Mix_CloseAudio();
		Mix_Quit();
		audio_context = NULL;
	}
}