#include "unit.hpp"


unsigned int unit_count;
Unit unit[UNIT_MAX];

void unit_init() {
    unit_count = 0;
}

unsigned int unit_create(uint8_t owner, vec2 position) {
    unsigned int index = unit_count;
    unit_count++;

    unit[index] = (Unit) {
        .owner = owner,
        .selected = false,
        .position = position
    };

    return index;
}