#include "render.hpp"

#include <stdio.h>

const int RENDER_TEXT_CENTERED = -1;

const SDL_Color COLOR_WHITE {
    .r = 255,
    .g = 255,
    .b = 255,
    .a = 255
};

const SDL_Color COLOR_YELLOW {
    .r = 255,
    .g = 255,
    .b = 0,
    .a = 255
};

TTF_Font* font_hack10pt;
TTF_Font* font_hack16pt;
TTF_Font* font_hack24pt;

bool render_init_fonts() {
    if (TTF_Init() == -1) {
        printf("Error initializing SDL_ttf: %s\n", TTF_GetError());
        return false;
    }

    font_hack10pt = TTF_OpenFont("./hack.ttf", 10);
    if (font_hack10pt == NULL) {
        printf("Unable to open hack.ttf! SDL Error: %s\n", TTF_GetError());
        return false;
    }
    font_hack16pt = TTF_OpenFont("./hack.ttf", 16);
    if (font_hack16pt == NULL) {
        printf("Unable to open hack.ttf! SDL Error: %s\n", TTF_GetError());
        return false;
    }
    font_hack24pt = TTF_OpenFont("./hack.ttf", 24);
    if (font_hack24pt == NULL) {
        printf("Unable to open hack.ttf! SDL Error: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void render_free_fonts() {
    TTF_CloseFont(font_hack10pt);
    TTF_CloseFont(font_hack16pt);
    TTF_CloseFont(font_hack24pt);

    TTF_Quit();
}

void render_text(std::string text, int x, int y, TTF_Font* font, SDL_Color color) {
    if (text == "") {
        return;
    }

    SDL_Surface* text_surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (text_surface == NULL) {
        printf("Unable to render text to surface! SDL Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (text_texture == NULL) {
        printf("Unable to create texture! SDL Error: %s\n", SDL_GetError());
        return;
    }

    SDL_Rect source_rect = (SDL_Rect) { .x = 0, .y = 0, .w = text_surface->w, .h = text_surface->h };
    SDL_Rect dest_rect = (SDL_Rect) { .x = x, .y = y, .w = text_surface->w, .h = text_surface->h };
    if (x == RENDER_TEXT_CENTERED) {
        dest_rect.x = (SCREEN_WIDTH / 2) - (text_surface->w / 2);
    }
    if (y == RENDER_TEXT_CENTERED) {
        dest_rect.y = (SCREEN_HEIGHT / 2) - (text_surface->h / 2);
    }
    SDL_RenderCopy(renderer, text_texture, &source_rect, &dest_rect);

    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);
}

SDL_Rect render_get_text_rect(std::string text, int x, int y, TTF_Font* font) {
    SDL_Surface* text_surface = TTF_RenderText_Solid(font, text.c_str(), COLOR_WHITE);
    if (text_surface == NULL) {
        printf("Unable to render text to surface! SDL Error: %s\n", TTF_GetError());
        return (SDL_Rect) { .x = 0, .y = 0, .w = 0, .h = 0 };
    }

    SDL_Rect source_rect = (SDL_Rect) { .x = 0, .y = 0, .w = text_surface->w, .h = text_surface->h };
    SDL_Rect dest_rect = (SDL_Rect) { .x = x, .y = y, .w = text_surface->w, .h = text_surface->h };
    if (x == RENDER_TEXT_CENTERED) {
        dest_rect.x = (SCREEN_WIDTH / 2) - (text_surface->w / 2);
    }
    if (y == RENDER_TEXT_CENTERED) {
        dest_rect.y = (SCREEN_HEIGHT / 2) - (text_surface->h / 2);
    }

    SDL_FreeSurface(text_surface);

    return dest_rect;
}