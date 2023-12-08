#include "match.hpp"

#include "network.hpp"
#include "map.hpp"

bool match_is_running;
bool match_is_started;

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
    }
}

void match_quit() {
    map_free();
}

void match_handle_input(SDL_Event e) {
    if (!match_is_running) {
        return;
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

    printf("started!\n");
}

void match_render() {
    if (!match_is_running) {
        return;
    }

    map_render();
}