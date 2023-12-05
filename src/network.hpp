#pragma once

#include <string>
#include <cstdint>

#include <enet/enet.h>

struct PlayerInfo {
    std::string username;
    ENetPeer* peer;
};

extern bool network_is_server;
extern bool network_is_connected;
extern uint8_t network_player_count;
extern uint8_t network_player_id;
extern PlayerInfo network_players[4];

std::string network_server_init(std::string p_username);
void network_server_disconnect();
void network_server_poll_events();
std::string network_client_init(std::string p_username);
void network_client_disconnect();
void network_client_poll_events();