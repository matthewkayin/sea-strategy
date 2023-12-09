#pragma once

#include "vector.hpp"

#include <cstdint>

const int UNIT_MAX = 128;

struct Unit {
    uint8_t owner;
    bool selected;
    vec2 position;
};

extern unsigned int unit_count;
extern Unit unit[UNIT_MAX];

void unit_init();
unsigned int unit_create(uint8_t owner, vec2 position);