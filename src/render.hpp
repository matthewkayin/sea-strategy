#pragma once

#include "vector.hpp"

#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

extern const int RENDER_TEXT_CENTERED;

extern const SDL_Color COLOR_WHITE;
extern const SDL_Color COLOR_YELLOW;

extern SDL_Renderer* renderer;

extern TTF_Font* font_hack10pt;
extern TTF_Font* font_hack16pt;
extern TTF_Font* font_hack24pt;

extern SDL_Texture* tileset_texture;
extern int tileset_tile_width;
extern int tileset_tile_height;

enum Texture {
    TEXTURE_SLOOP0,
    TEXTURE_SLOOP1,
    TEXTURE_SLOOP2,
    TEXTURE_SLOOP3,
    TEXTURE_COUNT
};

extern SDL_Texture* textures[TEXTURE_COUNT];
extern int texture_width[TEXTURE_COUNT];
extern int texture_height[TEXTURE_COUNT];

bool render_init_fonts();
void render_free_fonts();
void render_text(std::string text, int x, int y, TTF_Font* font, SDL_Color color);
SDL_Rect render_get_text_rect(std::string text, int x, int y, TTF_Font* font);

bool render_tileset_load(std::string path);
void render_tileset_free();
void render_tileset_tile(int tile_index, ivec2 position);

bool render_textures_init();
void render_textures_free();
void render_textures_render(Texture texture, ivec2 position);