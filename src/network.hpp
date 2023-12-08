#pragma once

#include "message.hpp"

#include <string>
#include <cstdint>
#include <queue>

#include <enet/enet.h>

struct PlayerInfo {
    std::string username;
    ENetPeer* peer;
    bool has_loaded;
};

extern bool network_is_server;
extern bool network_is_connected;
extern uint8_t network_player_count;
extern uint8_t network_player_id;
extern PlayerInfo network_players[4];

extern std::queue<Message> network_message_out_queue;
extern std::queue<Message> network_message_in_queue;

std::string network_server_init(std::string p_username);
void network_server_disconnect();
void network_server_poll_events();
std::string network_client_init(std::string p_username);
void network_client_disconnect();
void network_client_poll_events();
void network_flush_out_queue();