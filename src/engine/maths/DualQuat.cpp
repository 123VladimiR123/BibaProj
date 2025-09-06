#include "DualQuat.h"

DualQuat::DualQuat(Quat &&moved) noexcept {
    std::ranges::copy(moved.getReal(), std::begin(this->real));
    constexpr float img[4]{0.0f, 0.0f, 0.0f, 0.0f};
    this->setDual(img);
}
