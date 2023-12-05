#include "message.hpp"

#include <cstring>
#include <stdio.h>

std::string Message::serialize() {
    std::string data;
    data += (char)type;

    if (type == MESSAGE_TYPE_ASSIGN_ID) {
        data += (char)(assign_id.id);
    } else if (type == MESSAGE_TYPE_SET_USERNAME) {
        data += std::string(set_username.username);
    } else if (type == MESSAGE_TYPE_UPDATE_ROSTER) {
        data += (char)(update_roster.player_count);
        for (unsigned int i = 0; i < 4; i++) {
            data += std::string(update_roster.usernames[i]) + "\n";
        }
    }

    return data;
}

void Message::deserialize(char* data, size_t length) {
    type = (MessageType)data[0];

    if (type == MESSAGE_TYPE_ASSIGN_ID) {
        assign_id.id = (uint8_t)data[1];
    } else if (type == MESSAGE_TYPE_SET_USERNAME) {
        strcpy(set_username.username, data + 1);
    } else if (type == MESSAGE_TYPE_UPDATE_ROSTER) {
        update_roster.player_count = (uint8_t)data[1];
        std::string remaining_data = std::string(data + 2);
        for (unsigned int i = 0; i < 4; i++) {
            size_t delimit_pos = remaining_data.find('\n');
            strcpy(update_roster.usernames[i], remaining_data.substr(0, delimit_pos).c_str());
            remaining_data = remaining_data.substr(delimit_pos + 1);
        }
    }
}