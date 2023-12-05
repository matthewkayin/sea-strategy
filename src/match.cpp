#include "match.hpp"

#include "network.hpp"

bool match_is_running;

void match_init() {
    match_is_running = true;
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
}

void match_render() {
    if (!match_is_running) {
        return;
    }
}