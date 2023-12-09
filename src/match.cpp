#include "match.hpp"

#include "render.hpp"
#include "network.hpp"
#include "map.hpp"
#include "unit.hpp"

#include <algorithm>

const int SCREEN_DRAG_MARGIN = 20;
const float SCREEN_DRAG_SPEED = 100.0;

bool match_is_running;
bool match_is_started;

unsigned int game_tick;

vec2 screen_drag_direction;

ivec2 drag_start;
ivec2 drag_end;
bool is_dragging;

void match_init() {
    match_is_running = true;
    match_is_started = false;
    map_init("./map/map.tmx");

    for (uint8_t i = 0; i < 4; i++) {
        if (network_players[i].username == "") {
            continue;
        }
        unit_create(i, vec2(map_player_spawns[i] * 32));
    }

    is_dragging = false;

    if (!network_is_server) {
        network_message_out_queue.push((Message) {
            .type = MESSAGE_TYPE_LOAD_MATCH
        });
    } else {
        network_players[0].has_loaded = true;
        if (network_player_count == 1) {
            match_is_started = true;
        }
    }
}

void match_quit() {
    map_free();
}

void match_handle_input(SDL_Event e) {
    if (!match_is_started) {
        return;
    }

    if (e.type == SDL_MOUSEMOTION) {
        screen_drag_direction = vec2(0, 0);
        if (e.motion.x < SCREEN_DRAG_MARGIN) {
            screen_drag_direction.x = -1.0f;
        } else if (e.motion.x > SCREEN_WIDTH - SCREEN_DRAG_MARGIN) {
            screen_drag_direction.x = 1.0f;
        }
        if (e.motion.y < SCREEN_DRAG_MARGIN) {
            screen_drag_direction.y = -1.0f;
        } else if (e.motion.y > SCREEN_HEIGHT - SCREEN_DRAG_MARGIN) {
            screen_drag_direction.y = 1.0f;
        }
        if (is_dragging) {
            drag_end = map_camera_offset.to_ivec2() + ivec2(e.button.x, e.button.y);
        }
    } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        is_dragging = true;
        drag_start = map_camera_offset.to_ivec2() + ivec2(e.button.x, e.button.y);
        drag_end = drag_start;
    } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && is_dragging) {
        is_dragging = false;
        drag_end = map_camera_offset.to_ivec2() + ivec2(e.button.x, e.button.y);
        SDL_Rect drag_rect = (SDL_Rect) { .x = drag_start.x, .y = drag_start.y, .w = std::max(1, drag_end.x - drag_start.x), .h = std::max(1, drag_end.y - drag_start.y) };
        for (unsigned int i = 0; i < unit_count; i++) {
            if (unit[i].owner != network_player_id) {
                continue;
            }
            SDL_Rect unit_rect = (SDL_Rect) { .x = (int)(unit[i].position.x - map_camera_offset.x), .y = (int)(unit[i].position.y - map_camera_offset.y), .w = 32, .h = 32 };
            unit[i].selected = SDL_HasIntersection(&drag_rect, &unit_rect) == SDL_TRUE;
        }
    }
}

void match_update(float delta) {
    if (!match_is_running) {
        return;
    }

    // read and write network
    if (!network_is_connected) {
        match_is_running = false;
        return;
    }
    if (network_is_server) {
        network_server_poll_events();
    } else {
        network_client_poll_events();
    }

    while (!network_message_in_queue.empty()) {
        Message message = network_message_in_queue.front();
        network_message_in_queue.pop();

        if (network_is_server && message.type == MESSAGE_TYPE_LOAD_MATCH) {
            network_players[message.sender].has_loaded = true;
            bool all_players_are_loaded = true;
            for (unsigned i = 0; i < 4; i++) {
                if (network_players[i].username != "" && !network_players[i].has_loaded) {
                    all_players_are_loaded = false;
                    break;
                }
            }

            if (all_players_are_loaded) {
                match_is_started = true;
                network_message_out_queue.push((Message) {
                    .target = MESSAGE_TARGET_BROADCAST,
                    .type = MESSAGE_TYPE_START_MATCH
                });
            }
        } else if (!network_is_server && message.type == MESSAGE_TYPE_START_MATCH) {
            match_is_started = true;
        }
    }

    if (!match_is_started) {
        return;
    }

    map_camera_offset += screen_drag_direction * SCREEN_DRAG_SPEED * delta;
    map_camera_offset = map_camera_offset.clamp(vec2(0, 0), map_camera_limit);
    if (is_dragging) {
        ivec2 mouse_pos;
        SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        drag_end = map_camera_offset.to_ivec2() + mouse_pos;
    }
}

void match_render() {
    if (!match_is_running) {
        return;
    }

    map_render();

    SDL_Rect screen_rect = (SDL_Rect) { .x = (int)map_camera_offset.x, .y = (int)map_camera_offset.y, .w = SCREEN_WIDTH + 32, .h = SCREEN_HEIGHT + 32 };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (unsigned int i = 0; i < unit_count; i++) {
        SDL_Rect unit_rect = (SDL_Rect) { .x = (int)(unit[i].position.x - map_camera_offset.x), .y = (int)(unit[i].position.y - map_camera_offset.y), .w = 32, .h = 32 };
        if (SDL_HasIntersection(&screen_rect, &unit_rect) == SDL_FALSE) {
            continue;
        }
        render_textures_render((Texture)(TEXTURE_SLOOP0 + unit[i].owner), (unit[i].position - map_camera_offset).to_ivec2());
        if (unit[i].selected) {
            SDL_RenderDrawRect(renderer, &unit_rect);
        }
    }

    if (is_dragging) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        ivec2 drag_start_rendered = drag_start - map_camera_offset.to_ivec2();
        ivec2 drag_end_rendered = drag_end - map_camera_offset.to_ivec2();
        SDL_Rect drag_rect = (SDL_Rect) { .x = drag_start_rendered.x, .y = drag_start_rendered.y, .w = drag_end_rendered.x - drag_start_rendered.x, .h = drag_end_rendered.y - drag_start_rendered.y };
        SDL_RenderDrawRect(renderer, &drag_rect);
    }
}