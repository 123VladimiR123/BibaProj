#pragma once

#include <array>

#include "Util.h"

class Quat
{
protected:
    alignas(16) float real[4];

public:
    Quat(): Quat(std::array{0.0f,0.0f,0.0f,1.0f}.data()) {}

    explicit Quat(const float input[4]);

    void setReal(const float input[4]);

    [[nodiscard]] const float (& getReal() const)[4];

    float operator[](const int idx) const
    {
        return real[idx];
    }
};
