#pragma once

#include "vector.hpp"

#include <string>

extern unsigned int map_width;
extern unsigned int map_height;
extern unsigned int* map_tile;
extern ivec2 map_player_spawns[4];
extern ivec2 map_camera_offset;

bool map_init(std::string path);
void map_free();
unsigned int map_tile_at(ivec2 position);
void map_set_tile_at(ivec2 position, unsigned int value);
void map_render();