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

    Mat4 operator*(const Mat4& other) const {
        Mat4 result;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += data[row + k*4] * other.data[k + col*4];
                }
                result.data[row + col*4] = sum;
            }
        }
        return result;
    }
};
