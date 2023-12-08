#include "match.hpp"

#include "render.hpp"
#include "network.hpp"
#include "map.hpp"

#include <algorithm>

const int SCREEN_DRAG_MARGIN = 20;
const float SCREEN_DRAG_SPEED = 100.0;

bool match_is_running;
bool match_is_started;

vec2 screen_drag_direction;

void match_init() {
    match_is_running = true;
    match_is_started = false;
    map_init("./map/map.tmx");

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
}

void match_render() {
    if (!match_is_running) {
        return;
    }

    map_render();
}