#include "AudioContext.h"
#include "Paths.h"
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

	audio_context->music[0] = Mix_LoadMUS(MUSIC_1);
	audio_context->music[1] = Mix_LoadMUS(MUSIC_2);
	audio_context->music[2] = Mix_LoadMUS(MUSIC_3);
	audio_context->music[3] = Mix_LoadMUS(MUSIC_4);
	audio_context->music[4] = Mix_LoadMUS(MUSIC_5);
	audio_context->move_sound = Mix_LoadWAV(MOVE_SOUND);
	audio_context->lock_sound = Mix_LoadWAV(LOCK_SOUND);
	audio_context->clear_sound = Mix_LoadWAV(CLEAR_SOUND);
	audio_context->game_over = Mix_LoadWAV(GAME_OVER_SOUND);
	if (!audio_context->move_sound || !audio_context->lock_sound || !audio_context->clear_sound || !audio_context->game_over) {
		fprintf(stderr, "Error loading sound file: %s\n", Mix_GetError());
		destroy_audio_context();
		return false;
	}

	for (int i = 0; i < NUM_SONGS; i++) {
		if (!audio_context->music[i]) {
			fprintf(stderr, "Error loading audio file %d: %s\n", i, Mix_GetError());
			destroy_audio_context();
			return false;
		}
	}

	audio_context->music_paused = false;
	audio_context->sound_enabled = true;

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
		if (audio_context->music_paused) {
			Mix_PauseMusic();
		}
	}
}

void pause_music() {
	if (audio_context) {
		audio_context->music_paused = true;
		Mix_PauseMusic();
	}
}

void unpause_music() {
	if (audio_context) {
		audio_context->music_paused = false;
		Mix_ResumeMusic();
	}
}

void enable_sound() {
	if (audio_context) {
		audio_context->sound_enabled = true;
	}
}

void disable_sound() {
	if (audio_context) {
		audio_context->sound_enabled = false;
		Mix_HaltChannel(-1);
	}
}

void play_sound(Sound sound) {
	if (audio_context && audio_context->sound_enabled) {
		switch (sound) {
		case MOVE_SFX:
			Mix_PlayChannel(-1, audio_context->move_sound, 0);
			break;
		case LOCK_SFX:
			Mix_PlayChannel(-1, audio_context->lock_sound, 0);
			break;
		case CLEAR_SFX:
			Mix_PlayChannel(-1, audio_context->clear_sound, 0);
			break;
		case GAME_OVER_SFX:
			Mix_PlayChannel(-1, audio_context->game_over, 0);
			break;
		default:
			fprintf(stderr, "Invalid sound type\n");
			break;
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