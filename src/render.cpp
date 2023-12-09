#include "render.hpp"

#include <stdio.h>

#include <SDL2/SDL_image.h>

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

SDL_Texture* tileset_texture = NULL;
int tileset_tile_width;
int tileset_tile_height;

SDL_Texture* textures[TEXTURE_COUNT];
int texture_width[TEXTURE_COUNT];
int texture_height[TEXTURE_COUNT];

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

bool render_tileset_load(std::string path) {
    // render_tileset_free();

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == NULL) {
        printf("Unable to load tileset at %s. SDL Error: %s\n", path.c_str(), IMG_GetError());
        return false;
    }

    tileset_texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (tileset_texture == NULL) {
        printf("Unable to create texture from image. SDL Error: %s\n", SDL_GetError());
        return false;
    }

    tileset_tile_width = surface->w / 32;
    tileset_tile_height = surface->h / 32;

    SDL_FreeSurface(surface);

    return true;
}

void render_tileset_free() {
    if (tileset_texture != NULL) {
        SDL_DestroyTexture(tileset_texture);
        tileset_texture = NULL;
    }
}

void render_tileset_tile(int tile_index, ivec2 position) {
    SDL_Rect src_rect = (SDL_Rect) { .x = (tile_index % tileset_tile_width) * 32, .y = (int)((float)tile_index / (float)tileset_tile_width) * 32, .w = 32, .h = 32 };
    SDL_Rect dst_rect = (SDL_Rect) { .x = position.x, .y = position.y, .w = 32, .h = 32 };

    SDL_RenderCopy(renderer, tileset_texture, &src_rect, &dst_rect);
}

bool render_textures_init() {
    std::string texture_paths[TEXTURE_COUNT];
    texture_paths[TEXTURE_SLOOP0] = "./res/sloop0.png";
    texture_paths[TEXTURE_SLOOP1] = "./res/sloop1.png";
    texture_paths[TEXTURE_SLOOP2] = "./res/sloop2.png";
    texture_paths[TEXTURE_SLOOP3] = "./res/sloop3.png";

    for (unsigned int i = 0; i < TEXTURE_COUNT; i++) {
        SDL_Surface* surface = IMG_Load(texture_paths[i].c_str());
        if (surface == NULL) {
            printf("Unable to load texture at %s. SDL Error: %s\n", texture_paths[i].c_str(), IMG_GetError());
            return false;
        }

        textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        if (textures[i] == NULL) {
            printf("Unable to create texture from image. SDL Error: %s\n", SDL_GetError());
            return false;
        }

        texture_width[i] = surface->w;
        texture_height[i] = surface->h;

        SDL_FreeSurface(surface);
    }

    return true;
}

void render_textures_free() {
    for (unsigned int i = 0; i < TEXTURE_COUNT; i++) {
        SDL_DestroyTexture(textures[i]);
    }
}

void render_textures_render(Texture texture, ivec2 position) {
    SDL_Rect src_rect = (SDL_Rect) { .x = 0, .y = 0, .w = texture_width[texture], .h = texture_height[texture] };
    SDL_Rect dest_rect = (SDL_Rect) { .x = position.x, .y = position.y, .w = texture_width[texture], .h = texture_height[texture] };
    SDL_RenderCopy(renderer, textures[texture], &src_rect, &dest_rect);
}