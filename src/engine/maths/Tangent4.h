#pragma once
#include <cmath>

#include "Vector3.h"

struct Tangent4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr Tangent4() = default;

    constexpr Tangent4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w)
    {
    }

    constexpr Tangent4 operator+(const Tangent4& other) const
    {
        return {x + other.x, y + other.y, z + other.z, w + other.w};
    }

    constexpr Tangent4 operator-(const Tangent4& other) const
    {
        return {x - other.x, y - other.y, z - other.z, w - other.w};
    }

    constexpr Tangent4 operator*(const float scalar) const
    {
        return {x * scalar, y * scalar, z * scalar, w};
    }

    constexpr Tangent4 operator/(const float scalar) const
    {
        return {x / scalar, y / scalar, z / scalar, w};
    }

    Tangent4& operator+=(const Tangent4& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Tangent4& operator-=(const Tangent4& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    [[nodiscard]] float length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    [[nodiscard]] Tangent4 normalized() const
    {
        const float len = length();
        return len > 0.0f ? Tangent4{x / len, y / len, z / len, w} : *this;
    }

    [[nodiscard]] float dot(const Tangent4& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] Tangent4 cross(const Tangent4& other) const
    {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x,
            w
        };
    }

    [[nodiscard]] Vector3 toVector3() const
    {
        return {x, y, z};
    }
};
