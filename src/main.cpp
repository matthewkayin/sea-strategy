#ifdef _WIN32
    #define SDL_MAIN_HANDLED
#endif

#include "render.hpp"
#include "menu.hpp"
#include "match.hpp"
#include "network.hpp"

#include <stdio.h>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <enet/enet.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 360;

const unsigned long FRAME_TIME = 1000.0 / 60.0;
unsigned long last_time; 
unsigned long last_second; 
unsigned int frames = 0;
unsigned int fps = 0;

SDL_Window* window;
SDL_Renderer* renderer;

enum GameState {
    GAME_STATE_MENU = 0,
    GAME_STATE_MATCH = 1
};

GameState game_state;

bool game_init();
void game_quit();

int main() {
    if (!game_init()) {
        return 1;
    }

    menu_init();
    game_state = GAME_STATE_MENU;

    bool running = true;
    last_time = SDL_GetTicks();
    last_second = last_time; 
    while (running) {
        // Timekeep
        unsigned long current_time = SDL_GetTicks();
        if (current_time - last_time < FRAME_TIME) {
            continue;
        }

        float delta = (float)(current_time - last_time) / 60.0f;
        last_time = current_time;

        if (current_time - last_second >= 1000) {
            fps = frames;
            frames = 0;
            last_second += 1000;
        }

        // Handle input
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (SDL_GetWindowGrab(window) == SDL_FALSE) {
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    SDL_SetWindowGrab(window, SDL_TRUE);
                }
            } else {
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                    SDL_SetWindowGrab(window, SDL_FALSE);
                } else if (game_state == GAME_STATE_MENU) {
                    menu_handle_input(e);
                } else if (game_state == GAME_STATE_MATCH) {
                    match_handle_input(e);
                }
            }
        }

        if (game_state == GAME_STATE_MENU) {
            menu_update();
            if (!menu_is_running) {
                match_init();
                game_state = GAME_STATE_MATCH;
            }
        } else if (game_state == GAME_STATE_MATCH) {
            match_update(delta);
            if (!match_is_running) {
                match_quit();
                menu_init();
                game_state = GAME_STATE_MENU;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (game_state == GAME_STATE_MENU) {
            menu_render();
        } else if (game_state == GAME_STATE_MATCH) {
            match_render();
        }

        render_text("FPS: " + std::to_string(fps), 0, 0, font_hack10pt, COLOR_WHITE);

        SDL_RenderPresent(renderer);
        frames++;
    }

    game_quit();
    return 0;
}

bool game_init() {
    // init SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("eic", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Error creating window: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        printf("Error creating renderer: %s\n", SDL_GetError());
        return false;
    }

    // init fonts
    if (!render_init_fonts()) {
        return false;
    }

    if (enet_initialize() != 0) {
        printf("Unable to initialize enet\n");
        return false;
    }


    return true;
}

void game_quit() {
    if (game_state == GAME_STATE_MATCH) {
        match_quit();
    }

    enet_deinitialize();
    render_free_fonts();

    // quit SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}