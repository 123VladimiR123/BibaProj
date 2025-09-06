//
// Created by admin on 8/20/2025.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "./../../maths/Mat4.h"
#include "./../../maths/Util.h"
#include "./../nested/Transform.h"

class Camera
{
public:
    Transform transform;

    uint32_t nodeIdx{};

    //toDo should I cache this?
    [[nodiscard]] Mat4 getViewMat() { return Util::makeViewMatrix(transform.getQuat().value().getDual(), transform.getQuat().value().getReal()); }
    [[nodiscard]] virtual Mat4 getProjMat() const = 0;

    virtual ~Camera() = default;
};

#endif //CAMERA_H
