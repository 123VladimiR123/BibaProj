#include "Mat4.h"
#include <cstring>

Mat4::Mat4() {
    this->setIdentity();
}

Mat4::Mat4(const float data[16]) {
    std::memcpy(this->data, data, 16 * sizeof(float));
}

void Mat4::setIdentity() {
    alignas(16) static const float identity[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
    };
    std::memcpy(this->data, identity, 16 * sizeof(float));
}

const float *Mat4::raw() const
{
    return this->data;
}
