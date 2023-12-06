#pragma once

struct ivec2 {
    int x;
    int y;
    ivec2() {
        x = 0;
        y = 0;
    }
    ivec2(int x, int y) {
        this->x = x;
        this->y = y;
    }
    ivec2 operator+(const ivec2& other) const {
        return ivec2(x + other.x, y + other.y);
    }
    ivec2 operator-(const ivec2& other) const {
        return ivec2(x - other.x, y - other.y);
    }
    ivec2 operator*(const int scaler) const {
        return ivec2(x * scaler, y * scaler);
    }
};