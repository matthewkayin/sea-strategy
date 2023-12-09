#pragma once

#include <algorithm>

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

struct vec2 {
    float x;
    float y;
    vec2() {
        x = 0.0f;
        y = 0.0f;
    }
    vec2(float x, float y) {
        this->x = x;
        this->y = y;
    }
    vec2(const ivec2& other) {
        x = (float)other.x;
        y = (float)other.y;
    }
    ivec2 to_ivec2() const {
        return ivec2((int)x, (int)y);
    }
    vec2 operator+(const vec2& other) const {
        return vec2(x + other.x, y + other.y);
    }
    vec2 operator-(const vec2& other) const {
        return vec2(x - other.x, y - other.y);
    }
    vec2 operator*(const float scaler) const {
        return vec2(x * scaler, y * scaler);
    }
    vec2& operator+=(const vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    vec2& operator-=(const vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    vec2 clamp(const vec2& min, const vec2& max) const {
        return vec2(std::clamp(x, min.x, max.x), std::clamp(y, min.y, max.y));
    }
};