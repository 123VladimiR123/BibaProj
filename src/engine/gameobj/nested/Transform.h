#pragma once

#include "./../../maths/Mat4.h"
#include "./../../maths/Quat.h"
#include "./../../maths/DualQuat.h"
#include "./../../maths/Vector3.h"
#include <optional>

class Transform
{
    bool dirty{true};
    std::optional<DualQuat> quat; // w, x, y, z

public:
    Mat4 mtrx;
    std::optional<Vector3> scale;

    Transform();
    Transform(Transform&) = delete;

    Transform(Transform&& other) noexcept
        : mtrx(other.mtrx),
          quat(other.quat),
          scale(other.scale),
          dirty(other.dirty)
    {
    }

    Transform& operator=(Transform&) = delete;
    Transform& operator=(Transform&&) noexcept = default;

    Transform& operator=(Mat4& mat4)
    {
        mtrx = mat4;
        return *this;
    }

    Transform& operator=(Mat4&& mat4)
    {
        mtrx = mat4;
        return *this;
    }

    explicit Transform(DualQuat&& q);
    explicit Transform(DualQuat q);

    explicit Transform(Mat4&& mtrx);
    explicit Transform(Mat4& mtrx);

    void move(const float translation[3]);

    void rotate(const Quat& quatIn);

    void moveAndRotate(const Quat& quatIn, const float translation[3]);

    void emplaceFromMat4();

    //way to change dynamic transform
    std::optional<DualQuat>& getQuat();
    // cached way to get matrix
    Mat4& getMatrix();
};
