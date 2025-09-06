#pragma once

#include "Quat.h"
#include <utility>
#include <algorithm>
#include <iterator>

class DualQuat : public Quat {
protected:
    alignas(16) float dual[4];
public:
    explicit DualQuat(Quat &&moved) noexcept;

    void normFull() {
        Util::normFull(this->real, this->dual);
    }

    void setDual(const float input[4]) {
        _mm_store_ps(dual, _mm_loadu_ps(input));
    }

    [[nodiscard]] Mat4 getModelMat4(const float scale[3]) const
    {
        return Util::dualQuatToModelMat(this->real, this->dual, scale);
    }

    void translate(const float translation[3]) {
        Util::translate(this->real, this->dual, translation);
    }

    void rotate(const Quat &quat) {
        Util::rotate(this->real, this->dual, quat.getReal());
    }

    void translateAndRotate(const Quat &quat, const float translation[3]) {
        Util::translateAndRotate(this->real, this->dual, quat.getReal(), translation);
    }

    [[nodiscard]] float (&getDual())[4] {
        return this->dual;
    }
};