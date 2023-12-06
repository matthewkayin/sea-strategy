#pragma once

#include <SDL2/SDL.h>

extern bool match_is_running;

void match_init();
void match_quit();
void match_handle_input(SDL_Event e);
void match_update(float delta);
void match_render();