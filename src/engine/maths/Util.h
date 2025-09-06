#pragma once

#include <xmmintrin.h>
#include "Mat4.h"

class Util
{
    inline static __m128 loadToReg(const float* q);

    inline static void saveBack(float* q, __m128 reg);

public:
    static void normReal(float* real);

    static void orthDual(float* real, float* dual);

    static void normFull(float* real, float* dual);

    static Mat4 quatToMat4(const float* quat);

    static Mat4 dualQuatToModelMat(const float* r, const float* d, const float* sc);

    static Mat4 makeViewMatrix(const float* dual, const float* real);

    static Mat4 makeProjMatrixPerspective(float fovY, float aspect, float zNear, float zFar);

    static Mat4 makeProjMatrixOrtographic(float xmag, float ymag, float znear, float zfar);

    static void rotate(float real[4], float dual[4], const float rotation[4]);

    static void setTranslate(const float translation[3], const float real[4], float dual[4]);

    static void translate(float real[4], float dual[4], const float translation[3]);

    static void translateAndRotate(float real[4], float dual[4], const float rotation[4], const float translation[3]);

    static void modelMat4ToDualQuat(const Mat4& mat, float dual[4], float real[4], float scale[3]);

    static void fromVec3Rot3ToDualQuat(const float t[3], const float r[3], float dual[4], float real[4]);
};
