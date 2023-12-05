#include "menu.hpp"

#include "render.hpp"
#include "network.hpp"

#include <string>

struct Textbox {
    SDL_Rect rect;
    std::string text;
    void init(unsigned int charwidth, std::string default_text, int start_x, int start_y) {
        rect = render_get_text_rect(std::string(charwidth, 'X'), start_x, start_y, font_hack16pt);
        rect.x -= 2;
        rect.y -= 2;
        rect.w += 4;
        rect.h += 4;
        text = default_text;
    }
    void render(bool is_hovered)  {
        if (is_hovered) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
        SDL_RenderDrawRect(renderer, &rect);
        render_text(text, rect.x + 2, rect.y + 2, font_hack16pt, is_hovered ? COLOR_YELLOW : COLOR_WHITE);
    }
};

enum MenuIndex {
    MENU_INDEX_NONE = -1,
    MENU_INDEX_HOST = 0,
    MENU_INDEX_JOIN = 1,
    MENU_INDEX_USERNAME = 2,
    MENU_INDEX_IPADDR = 3,
    MENU_INDEX_BACK = 4,
    MENU_INDEX_START = 5,
};

enum MenuState {
    MENU_STATE_MAIN = 0,
    MENU_STATE_LOBBY = 1
};

SDL_Rect host_button_rect;
SDL_Rect join_button_rect;
Textbox textbox_username;
Textbox textbox_ipaddr;

SDL_Rect back_button_rect;
SDL_Rect start_button_rect;

MenuState menu_state;
MenuIndex menu_index;
MenuIndex text_input_state;
std::string error_message;

bool menu_is_running = false;

void menu_init() {
    host_button_rect = render_get_text_rect("HOST", RENDER_TEXT_CENTERED, 264, font_hack16pt);
    join_button_rect = render_get_text_rect("JOIN", RENDER_TEXT_CENTERED, 296, font_hack16pt);

    textbox_username.init(10, "", RENDER_TEXT_CENTERED, 200);
    textbox_ipaddr.init(10, "127.0.0.1", RENDER_TEXT_CENTERED, 232);

    back_button_rect = render_get_text_rect("BACK", 64, 264, font_hack16pt);
    start_button_rect = render_get_text_rect("START", back_button_rect.x + back_button_rect.w + 32, 264, font_hack16pt);

    text_input_state = MENU_INDEX_NONE;
    menu_index = MENU_INDEX_NONE;
    menu_state = MENU_STATE_MAIN;

    error_message = "";

    menu_is_running = true;
}

void menu_handle_input(SDL_Event e) {
    if (!menu_is_running) {
        return;
    }

    if (menu_state == MENU_STATE_MAIN) {
        if (e.type == SDL_MOUSEMOTION) {
            SDL_Point mouse_position = (SDL_Point) { .x = e.motion.x, .y = e.motion.y };
            if (SDL_PointInRect(&mouse_position, &textbox_username.rect)) {
                menu_index = MENU_INDEX_USERNAME;
            } else if (SDL_PointInRect(&mouse_position, &textbox_ipaddr.rect)) {
                menu_index = MENU_INDEX_IPADDR;
            } else if (SDL_PointInRect(&mouse_position, &host_button_rect)) {
                menu_index = MENU_INDEX_HOST;
            } else if (SDL_PointInRect(&mouse_position, &join_button_rect)) {
                menu_index = MENU_INDEX_JOIN;
            } else {
                menu_index = MENU_INDEX_NONE;
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            if (text_input_state != MENU_INDEX_NONE) {
                SDL_StopTextInput();
                text_input_state = MENU_INDEX_NONE;
            }
            if (menu_index == MENU_INDEX_USERNAME || menu_index == MENU_INDEX_IPADDR) {
                SDL_StartTextInput();
                text_input_state = menu_index;
            } else if (menu_index == MENU_INDEX_HOST) {
                if (textbox_username.text == "") {
                    error_message = "Please enter a username.";
                    return;
                }
                error_message = network_server_init(textbox_username.text);
                if (error_message != "") {
                    return;
                }
                menu_state = MENU_STATE_LOBBY;
            } else if (menu_index == MENU_INDEX_JOIN) {
                if (textbox_username.text == "") {
                    error_message = "Please enter a username.";
                    return;
                }
                if (textbox_ipaddr.text == "") {
                    error_message = "Please enter an IP address.";
                    return;
                }
                error_message = network_client_init(textbox_username.text);
                if (error_message != "") {
                    return;
                }
                menu_state = MENU_STATE_LOBBY;
            }
        } else if (e.type == SDL_TEXTINPUT) {
            if (text_input_state == MENU_INDEX_USERNAME) {
                textbox_username.text += e.text.text;
            } else if (text_input_state == MENU_INDEX_IPADDR) {
                textbox_ipaddr.text += e.text.text;
            }
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_BACKSPACE) {
                if (text_input_state == MENU_INDEX_USERNAME && textbox_username.text.length() > 0) {
                    textbox_username.text.pop_back();
                } else if (text_input_state == MENU_INDEX_IPADDR && textbox_ipaddr.text.length() > 0) {
                    textbox_ipaddr.text.pop_back();
                }
            }
        }
    } else if (menu_state == MENU_STATE_LOBBY) {
        if (e.type == SDL_MOUSEMOTION) {
            SDL_Point mouse_position = (SDL_Point) { .x = e.motion.x, .y = e.motion.y };
            if (SDL_PointInRect(&mouse_position, &back_button_rect)) {
                menu_index = MENU_INDEX_BACK;
            } else if (network_is_server && SDL_PointInRect(&mouse_position, &start_button_rect)) {
                menu_index = MENU_INDEX_START;
            } else {
                menu_index = MENU_INDEX_NONE;
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            if (menu_index == MENU_INDEX_BACK) {
                if (!network_is_server) {
                    network_client_disconnect();
                } else {
                    network_server_disconnect();
                }
                menu_state = MENU_STATE_MAIN;
            }
        }
    }
}

void menu_update() {
    if (!menu_is_running) {
        return;
    }

    if (menu_state == MENU_STATE_LOBBY) {
        if (!network_is_connected) {
            menu_state = MENU_STATE_MAIN;
            error_message = "The host disconnected";
            return;
        }
        if (network_is_server) {
            network_server_poll_events();
        } else {
            network_client_poll_events();
        }
    }
}

void menu_render() {
    if (!menu_is_running) {
        return;
    }

    render_text("BLOCKADE", RENDER_TEXT_CENTERED, 84, font_hack24pt, COLOR_WHITE);
    if (menu_state == MENU_STATE_MAIN) {
        render_text(error_message, RENDER_TEXT_CENTERED, 136, font_hack16pt, COLOR_WHITE);
        textbox_username.render(menu_index == MENU_INDEX_USERNAME);
        textbox_ipaddr.render(menu_index == MENU_INDEX_IPADDR);
        render_text("HOST", host_button_rect.x, host_button_rect.y, font_hack16pt, menu_index == MENU_INDEX_HOST ? COLOR_YELLOW : COLOR_WHITE);
        render_text("JOIN", join_button_rect.x, join_button_rect.y, font_hack16pt, menu_index == MENU_INDEX_JOIN ? COLOR_YELLOW : COLOR_WHITE);
    } else if (menu_state == MENU_STATE_LOBBY) {
        for (unsigned int i = 0; i < 4; i++) {
            render_text(network_players[i].username, 64, 136 + (32 * i), font_hack16pt, COLOR_WHITE);
        }

        render_text("BACK", back_button_rect.x, back_button_rect.y, font_hack16pt, menu_index == MENU_INDEX_BACK ? COLOR_YELLOW : COLOR_WHITE);
        if (network_is_server) {
            render_text("START", start_button_rect.x, start_button_rect.y, font_hack16pt, menu_index == MENU_INDEX_START ? COLOR_YELLOW : COLOR_WHITE);
        }
    }
}