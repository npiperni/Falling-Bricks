#pragma once

bool setup();

void cleanup();

void process_input(bool* running);

void update();

void render(SDL_Renderer* renderer);