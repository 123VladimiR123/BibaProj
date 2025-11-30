#pragma once

class Mat4
{
public:
    alignas(16) float data[16]; // column major

    Mat4();

    explicit Mat4(const float data[16]); // must be alignas
    void setIdentity();

    [[nodiscard]] const float* raw() const;

    float& operator()(const int col, const int row) {
        return this->data[row * 4 + col];
    }

    float operator()(const int col, const int row) const {
        return this->data[row * 4 + col];
    }
};
