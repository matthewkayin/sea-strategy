#pragma once

#include <string>
#include <cstdint>

extern bool network_is_server;
extern uint8_t network_player_count;
extern uint8_t network_player_id;
extern std::string network_players[4];

std::string network_server_init(std::string p_username);
void network_server_poll_events();
std::string network_client_init(std::string p_username);
void network_client_disconnect();
void network_client_poll_events();