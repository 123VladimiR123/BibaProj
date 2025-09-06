#pragma once
#include <cmath>

struct Vector4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    constexpr Vector4() = default;

    constexpr Vector4(const float x, const float y, const float z, const float w)
        : x(x), y(y), z(z), w(w)
    {
    }

    constexpr Vector4 operator+(const Vector4& other) const
    {
        return {x + other.x, y + other.y, z + other.z, w + other.w};
    }

    constexpr Vector4 operator-(const Vector4& other) const
    {
        return {x - other.x, y - other.y, z - other.z, w - other.w};
    }

    constexpr Vector4 operator*(const float scalar) const
    {
        return {x * scalar, y * scalar, z * scalar, w * scalar};
    }

    constexpr Vector4 operator/(const float scalar) const
    {
        return {x / scalar, y / scalar, z / scalar, w / scalar};
    }

    Vector4& operator+=(const Vector4& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Vector4& operator-=(const Vector4& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    [[nodiscard]] float length() const
    {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    [[nodiscard]] Vector4 normalized() const
    {
        const float len = length();
        return len > 0.0f ? *this / len : *this;
    }

    [[nodiscard]] float dot(const Vector4& other) const
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }
};
