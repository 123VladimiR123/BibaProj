//
// Created by admin on 8/25/2025.
//

#ifndef PERSPECTIVECAMERA_H
#define PERSPECTIVECAMERA_H
#include "Camera.h"
#include "./../../maths/Mat4.h"
#include "./../../maths/Util.h"

class PerspectiveCamera final : public Camera
{
public:
    float fov = 45.0f;
    float aspect = 1.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    [[nodiscard]] Mat4 getProjMat() const override { return Util::makeProjMatrixPerspective(fov, aspect, nearPlane, farPlane); }

    ~PerspectiveCamera() override = default;
};

#endif //PERSPECTIVECAMERA_H
