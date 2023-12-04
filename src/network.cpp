#include "network.hpp"

#include <stdio.h>
#include <cstdint>

#include <enet/enet.h>

enum MessageType {
    MESSAGE_TYPE_ASSIGN_ID = 0,
    MESSAGE_TYPE_SET_USERNAME = 1,
    MESSAGE_TYPE_UPDATE_ROSTER = 2
};

struct MessageAssignId {
    uint8_t id;
    std::string serialize() {
        std::string data;
        data += (char)MESSAGE_TYPE_ASSIGN_ID;
        data += (char)id;

        return data;
    }
    void deserialize(char* data, size_t length) {
        id = (uint8_t)data[0];
    }
};

struct MessageSetUsername {
    char username[12];
    std::string serialize() {
        std::string data;
        data += (char)MESSAGE_TYPE_SET_USERNAME;
        data += std::string(username);

        return data;
    }
    void deserialize(char* data, size_t length) {
        strcpy(username, data);
    }
};

struct MessageUpdateRoster {
    char usernames[4][12];
    uint8_t player_count;
    std::string serialize() {
        std::string data;
        data += (char)MESSAGE_TYPE_UPDATE_ROSTER;
        data += (char)player_count;
        for (unsigned int i = 0; i < 4; i++) {
            data += std::string(usernames[i]) + "\n";
        }

        return data;
    }
    void deserialize(char* data, size_t length) {
        player_count = (uint8_t)data[0];
        unsigned int base = 1;
        for (unsigned int i = 0; i < 4; i++) {
            usernames[i][0] = '\0';
            unsigned int index = 0;
            while (base + index < length && data[base + index] != '\n') {
                usernames[i][index] = data[base + index];
                index++;
            }
            base = base + index + 1;
        }
    }
};

struct Message {
    MessageType type;
    union {
        MessageAssignId assign_id;
        MessageSetUsername set_username;
        MessageUpdateRoster update_roster;
    };
    Message(char* data, size_t length) {
        type = (MessageType)data[0];
        switch (type) {
            case MESSAGE_TYPE_ASSIGN_ID:
                assign_id.deserialize(data + 1, length);
                break;
            case MESSAGE_TYPE_SET_USERNAME:
                set_username.deserialize(data + 1, length);
                break;
            case MESSAGE_TYPE_UPDATE_ROSTER:
                update_roster.deserialize(data + 1, length);
                break;
            default:
                printf("Message type of %u not recognized.\n", type);
                break;
        }
    }
};

bool network_is_server = false;

ENetAddress address;
ENetHost* host;
ENetPeer* client_peer;

std::string username;
std::string network_players[4];
uint8_t network_player_count = 0;
uint8_t network_player_id;

std::string network_server_init(std::string p_username) {
    for (unsigned int i = 0; i < 4; i++) {
        network_players[i] = "";
    }

    network_players[0] = p_username.substr(0, 12);
    network_player_count = 1;
    network_player_id = 0;

    address.host = ENET_HOST_ANY;
    address.port = 6700;
    host = enet_host_create(&address, 3, 2, 0, 0);
    if (host == NULL) {
        return "Error initializing server host.";
    }

    network_is_server = true;
    return "";
}

void network_server_poll_events() {
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            printf("A new client connnected %x:%u.\n", event.peer->address.host, event.peer->address.port);
            for (unsigned int i = 0; i < 4; i++) {
                if (network_players[i] == "") {
                    event.peer->data = new uint8_t(i);
                    break;
                }
            }
            network_player_count++;

            MessageAssignId message = (MessageAssignId) {
                .id = *((uint8_t*)event.peer->data)
            };
            std::string message_string = message.serialize();
            ENetPacket* packet = enet_packet_create(message_string.c_str(), message_string.length() + 1, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(event.peer, 0, packet);
            enet_host_flush(host);
        } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            Message message((char*)event.packet->data, event.packet->dataLength);

            if (message.type == MESSAGE_TYPE_SET_USERNAME) {
                network_players[*((uint8_t*)event.peer->data)] = message.set_username.username;

                MessageUpdateRoster roster_message; 
                roster_message.player_count = network_player_count;
                for (unsigned int i = 0; i < 4; i++) {
                    strcpy(roster_message.usernames[i], network_players[i].c_str());
                }
                std::string message_string = roster_message.serialize();
                ENetPacket* packet = enet_packet_create(message_string.c_str(), message_string.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(host, 0, packet);
                enet_host_flush(host);
            }

            enet_packet_destroy(event.packet);
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            printf("Client disconnected\n");
            network_players[*((uint8_t*)event.peer->data)] = "";
            delete (uint8_t*)event.peer->data;
            event.peer->data = NULL;

            MessageUpdateRoster roster_message; 
            roster_message.player_count = network_player_count;
            for (unsigned int i = 0; i < 4; i++) {
                strcpy(roster_message.usernames[i], network_players[i].c_str());
            }
            std::string message_string = roster_message.serialize();
            ENetPacket* packet = enet_packet_create(message_string.c_str(), message_string.length() + 1, ENET_PACKET_FLAG_RELIABLE);
            enet_host_broadcast(host, 0, packet);
            enet_host_flush(host);
        }
    }
}

std::string network_client_init(std::string p_username) {
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
    return "";
}

void network_client_disconnect() {
    enet_peer_disconnect(client_peer, 0);

    ENetEvent event;
    while (enet_host_service(host, &event, 3000) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            enet_packet_destroy(event.packet);
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            printf("Disconnection succeeded\n");
            return;
        }
    }

    enet_peer_reset(client_peer);
}

void network_client_poll_events() {
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            Message message((char*)event.packet->data, event.packet->dataLength);

            if (message.type == MESSAGE_TYPE_ASSIGN_ID) {
                network_player_id = message.assign_id.id;
                network_players[network_player_id] = username;

                MessageSetUsername message;
                strcpy(message.username, username.c_str());
                std::string message_serialized = message.serialize();
                ENetPacket* packet = enet_packet_create(message_serialized.c_str(), message_serialized.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(client_peer, 0, packet);
                enet_host_flush(host);
            } else if (message.type == MESSAGE_TYPE_UPDATE_ROSTER) {
                network_player_count = message.update_roster.player_count;
                printf("player count %u\n", network_player_count);
                for (unsigned int i = 0; i < 4; i++) {
                    network_players[i] = std::string(message.update_roster.usernames[i]);
                    printf("%s\n", network_players[i].c_str());
                }
            }

            enet_packet_destroy(event.packet);
        }
    }
}