#include "map.hpp"

#include "render.hpp"

#include <fstream>
#include <stdio.h>
#include <map>

const unsigned int SCREEN_TILE_WIDTH = (unsigned int)(SCREEN_WIDTH / 32.0) + 1;
const unsigned int SCREEN_TILE_HEIGHT = (unsigned int)(SCREEN_HEIGHT / 32.0) + 1;

unsigned int map_width;
unsigned int map_height;
unsigned int* map_tile;
ivec2 map_player_spawns[4];
vec2 map_camera_offset;
vec2 map_camera_limit;

struct XMLTag {
    std::string name;
    std::map<std::string, std::string> data;
    XMLTag(std::string line) {
        while (line[0] == ' ' || line[0] == '<' || line[0] == '?') {
            line.erase(0, 1);
        }
        while (line[line.length() - 1] == '>' || line[line.length() - 1] == '/') {
            line.pop_back();
        }

        size_t space_index = line.find(' ');
        name = line.substr(0, space_index);
        line = space_index == std::string::npos ? "" : line.substr(space_index + 1);

        while (line != "") {
            size_t equals_index = line.find('=');
            std::string key = line.substr(0, line.find('='));
            line = line.substr(equals_index + 2);
            size_t quote_index = line.find('"');
            std::string value = line.substr(0, quote_index);
            line = quote_index + 2 > line.length() - 1 ? "" : line.substr(quote_index + 2);

            data[key] = value;
        }
    }
};

bool map_init(std::string path) {
    map_free();

    std::string line;
    std::ifstream map_file(path);
    if (!map_file.is_open()) {
        return false;
    }

    bool in_tile_layer = false;
    unsigned int tile_offset;
    bool in_spawn_layer = false;
    unsigned int spawn_offset;
    ivec2 layer_pos;

    while (std::getline(map_file, line)) {
        bool is_line_xml_tag = line.find('<') != std::string::npos;
        if (is_line_xml_tag) {
            XMLTag xml_tag(line);
            if (xml_tag.name == "tileset" && xml_tag.data["source"] == "eic_maptiles.tsx") {
                render_tileset_load("./res/tiles.png");
                tile_offset = stoul(xml_tag.data["firstgid"]);
            } else if (xml_tag.name == "tileset" && xml_tag.data["source"] == "eic_spawns.tsx") {
                spawn_offset = stoul(xml_tag.data["firstgid"]);
            } else if (xml_tag.name == "layer" && xml_tag.data["name"] == "tile") {
                in_tile_layer = true;
                map_width = stoul(xml_tag.data["width"]);
                map_height = stoul(xml_tag.data["height"]);
                map_camera_limit = vec2((map_width * 32.0f) - SCREEN_WIDTH, (map_height * 32.0f) - SCREEN_HEIGHT);
                map_tile = new unsigned int[map_width * map_height];
                layer_pos = ivec2(0, 0);
            } else if (xml_tag.name == "layer" && xml_tag.data["name"] == "tile") {
                in_spawn_layer = true;
                layer_pos = ivec2(0, 0);
            } else if (xml_tag.name == "/layer") {
                in_tile_layer = false;
                in_spawn_layer = false;
            }
        } else {
            while (line != "") {
                size_t delimit_index = line.find(',');
                unsigned int next_value;
                if (delimit_index != std::string::npos) {
                    next_value = stoul(line.substr(0, delimit_index));
                    line = line.substr(delimit_index + 1);
                } else {
                    // if there's no comma, then we're at the last value of the last line of this layer
                    next_value = stoul(line);
                    line = "";
                }

                if (in_tile_layer) {
                    map_set_tile_at(layer_pos, next_value - tile_offset);
                } else if (in_spawn_layer) {
                    unsigned int player_index = next_value - spawn_offset;
                    map_player_spawns[player_index] = layer_pos;
                }

                layer_pos.x++;
            }
            layer_pos.y++;
            layer_pos.x = 0;
        }
    }

    map_file.close();

    render_tileset_load("./res/tiles.png");

    map_camera_offset = vec2(0, 0);

    return true;
}

void map_free() {
    if (map_tile != NULL) {
        delete [] map_tile;
        map_tile = NULL;

        render_tileset_free();
    }
}

unsigned int map_tile_at(ivec2 position) {
    return map_tile[position.x + (position.y * map_width)];
}

void map_set_tile_at(ivec2 position, unsigned int value) {
    map_tile[position.x + (position.y * map_width)] = value;
}

void map_render() {
    ivec2 top_left_cell = ivec2((int)(map_camera_offset.x / 32.0), (int)(map_camera_offset.y / 32.0));
    ivec2 map_camera_offset_as_ivec2 = map_camera_offset.to_ivec2();
    for (unsigned int x = top_left_cell.x; x < top_left_cell.x + SCREEN_TILE_WIDTH; x++) {
        for (unsigned int y = top_left_cell.y; y < top_left_cell.y + SCREEN_TILE_HEIGHT; y++) {
            ivec2 tile_cell = ivec2(x, y);
            ivec2 render_position = (tile_cell * 32) - map_camera_offset_as_ivec2;
            render_tileset_tile(map_tile_at(tile_cell), render_position);
        }
    }
}