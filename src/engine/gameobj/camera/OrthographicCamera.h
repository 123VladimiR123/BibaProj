//
// Created by admin on 8/25/2025.
//

#ifndef ORTHOGRAPHICCAMERA_H
#define ORTHOGRAPHICCAMERA_H
#include "Camera.h"
#include "./../../maths/Mat4.h"
#include "./../../maths/Util.h"

class OrthographicCamera final : public Camera
{
public:
    float xmag = 1.0f;
    float ymag = 1.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    [[nodiscard]] Mat4 getProjMat() const override { return Util::makeProjMatrixOrtographic(xmag, ymag, nearPlane, farPlane); }

    ~OrthographicCamera() override = default;
};

#endif //ORTHOGRAPHICCAMERA_H
