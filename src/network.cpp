#include "network.hpp"

#include "message.hpp"

#include <stdio.h>
#include <cstdint>

#include <enet/enet.h>

bool network_is_server = false;
bool network_is_connected = false;

ENetAddress address;
ENetHost* host;
ENetPeer* client_peer;

std::string username;
PlayerInfo network_players[4];
uint8_t network_player_count = 0;
uint8_t network_player_id;

std::string network_server_init(std::string p_username) {
    for (unsigned int i = 0; i < 4; i++) {
        network_players[i].username = "";
        network_players[i].peer = NULL;
    }

    network_players[0].username = p_username.substr(0, 12);
    network_player_count = 1;
    network_player_id = 0;

    address.host = ENET_HOST_ANY;
    address.port = 6700;
    host = enet_host_create(&address, 3, 2, 0, 0);
    if (host == NULL) {
        return "Error initializing server host.";
    }

    network_is_server = true;
    network_is_connected = true;
    return "";
}

void network_server_disconnect() {
    for (unsigned int i = 1; i < 4; i++) {
        if (network_players[i].username == "") {
            continue;
        }
        enet_peer_disconnect(network_players[i].peer, 0);
    }

    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 3000) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            enet_packet_destroy(event.packet);
        } else if (event.type == ENET_EVENT_TYPE_CONNECT) {
            enet_peer_disconnect(event.peer, 0);
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            uint8_t peer_id = *((uint8_t*)event.peer->data);
            network_players[peer_id].username = "";
            network_players[peer_id].peer = NULL;
        }
    }

    for (unsigned int i = 1; i < 4; i++) {
        if (network_players[i].username != "") {
            enet_peer_reset(network_players[i].peer);
            network_players[i].username = "";
            network_players[i].peer = NULL;
        }
    }

    enet_host_destroy(host);

    network_players[0].username = "";
    network_is_server = false;
    network_is_connected = false;
}

void network_server_poll_events() {
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            for (unsigned int i = 1; i < 4; i++) {
                if (network_players[i].username == "") {
                    event.peer->data = new uint8_t(i);
                    network_players[i].peer = event.peer;
                    break;
                }
            }
            network_player_count++;

            Message message = (Message) {
                .type = MESSAGE_TYPE_ASSIGN_ID,
                .assign_id = {
                    .id = *((uint8_t*)event.peer->data)
                }
            };
            std::string message_string = message.serialize();
            ENetPacket* packet = enet_packet_create(message_string.c_str(), message_string.length() + 1, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(event.peer, 0, packet);
            enet_host_flush(host);
        } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            Message message;
            message.deserialize((char*)event.packet->data, event.packet->dataLength);

            if (message.type == MESSAGE_TYPE_SET_USERNAME) {
                network_players[*((uint8_t*)event.peer->data)].username = message.set_username.username;

                Message roster_message; 
                roster_message.type = MESSAGE_TYPE_UPDATE_ROSTER;
                roster_message.update_roster.player_count = network_player_count;
                for (unsigned int i = 0; i < 4; i++) {
                    strcpy(roster_message.update_roster.usernames[i], network_players[i].username.c_str());
                }
                std::string message_string = roster_message.serialize();
                ENetPacket* packet = enet_packet_create(message_string.c_str(), message_string.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(host, 0, packet);
                enet_host_flush(host);
            }

            enet_packet_destroy(event.packet);
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            network_players[*((uint8_t*)event.peer->data)].username = "";
            network_players[*((uint8_t*)event.peer->data)].peer = NULL;
            delete (uint8_t*)event.peer->data;
            event.peer->data = NULL;

            Message roster_message;
            roster_message.type = MESSAGE_TYPE_UPDATE_ROSTER;
            roster_message.update_roster.player_count = network_player_count;
            for (unsigned int i = 0; i < 4; i++) {
                strcpy(roster_message.update_roster.usernames[i], network_players[i].username.c_str());
            }
            std::string message_string = roster_message.serialize();
            ENetPacket* packet = enet_packet_create(message_string.c_str(), message_string.length() + 1, ENET_PACKET_FLAG_RELIABLE);
            enet_host_broadcast(host, 0, packet);
            enet_host_flush(host);
        }
    }
}

std::string network_client_init(std::string p_username) {
    for (unsigned int i = 0; i < 4; i++) {
        network_players[i].username = "";
        network_players[i].peer = NULL;
    }

    username = p_username;

    host = enet_host_create(NULL, 1, 2, 0, 0);
    if (host == NULL) {
        return "Error initializing client host.";
    }

    enet_address_set_host(&address, "127.0.0.1");
    address.port = 6700;
    client_peer = enet_host_connect(host, &address, 2, 0);
    if (client_peer == NULL) {
        return "No available peers for initiating ENet connection.";
    }

    ENetEvent event;
    bool connection_success = enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT;
    if (!connection_success) {
        return "Failed to connect to server.";
    }

    enet_host_flush(host);
    network_is_connected = true;
    network_is_server = false;
    return "";
}

void network_client_disconnect() {
    enet_peer_disconnect(client_peer, 0);

    ENetEvent event;
    while (enet_host_service(host, &event, 3000) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            enet_packet_destroy(event.packet);
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            return;
        }
    }

    enet_peer_reset(client_peer);
    enet_host_destroy(host);

    network_is_connected = false;
}

void network_client_poll_events() {
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            enet_peer_reset(client_peer);
            enet_host_destroy(host);
            network_is_connected = false;
        } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            Message message;
            message.deserialize((char*)event.packet->data, event.packet->dataLength);

            if (message.type == MESSAGE_TYPE_ASSIGN_ID) {
                network_player_id = message.assign_id.id;
                network_players[network_player_id].username = username;

                Message username_message;
                username_message.type = MESSAGE_TYPE_SET_USERNAME;
                strcpy(username_message.set_username.username, username.c_str());
                std::string message_serialized = username_message.serialize();
                ENetPacket* packet = enet_packet_create(message_serialized.c_str(), message_serialized.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(client_peer, 0, packet);
                enet_host_flush(host);
            } else if (message.type == MESSAGE_TYPE_UPDATE_ROSTER) {
                network_player_count = message.update_roster.player_count;
                for (unsigned int i = 0; i < 4; i++) {
                    network_players[i].username = std::string(message.update_roster.usernames[i]);
                }
            }

            enet_packet_destroy(event.packet);
        }
    }
}