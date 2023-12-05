#pragma once

#include <SDL2/SDL.h>

extern bool menu_is_running;

void menu_init();
void menu_handle_input(SDL_Event e);
void menu_update();
void menu_render();