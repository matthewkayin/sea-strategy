#pragma once

#include <cstdint>
#include <string>

#include <enet/enet.h>

const uint8_t MESSAGE_TARGET_BROADCAST = 0;

enum MessageType {
    MESSAGE_TYPE_ASSIGN_ID = 0,
    MESSAGE_TYPE_SET_USERNAME = 1,
    MESSAGE_TYPE_UPDATE_ROSTER = 2,
    MESSAGE_TYPE_LOAD_MATCH = 3,
    MESSAGE_TYPE_START_MATCH = 4
};

struct MessageAssignId {
    uint8_t id;
};

struct MessageSetUsername {
    char username[12];
};

struct MessageUpdateRoster {
    char usernames[4][12];
    uint8_t player_count;
};

struct Message {
    union {
        uint8_t target;
        uint8_t sender;
    };
    MessageType type;
    union {
        MessageAssignId assign_id;
        MessageSetUsername set_username;
        MessageUpdateRoster update_roster;
    };
    std::string serialize();
    void deserialize(char* data, size_t length);
};