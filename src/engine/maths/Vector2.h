#pragma once
#include <cmath>

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vector2() = default;
    constexpr Vector2(const float x, const float y) : x(x), y(y) {}

    // арифметика
    constexpr Vector2 operator+(const Vector2& other) const {
        return {x + other.x, y + other.y};
    }

    constexpr Vector2 operator-(const Vector2& other) const {
        return {x - other.x, y - other.y};
    }

    constexpr Vector2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }

    constexpr Vector2 operator/(float scalar) const {
        return {x / scalar, y / scalar};
    }

    bool operator==(const Vector2& other) const
    {
        return x == other.x && y == other.y;
    }

    // todo intrisicts
    Vector2& operator+=(const Vector2& other) {
        x += other.x; y += other.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& other) {
        x -= other.x; y -= other.y;
        return *this;
    }

    [[nodiscard]] float length() const {
        return std::sqrt(x * x + y * y);
    }

    [[nodiscard]] Vector2 normalized() const {
        const float len = length();
        return len > 0.0f ? *this / len : *this;
    }

    [[nodiscard]] float dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }
};