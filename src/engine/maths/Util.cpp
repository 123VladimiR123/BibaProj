#include "Util.h"

#include <array>
#include <smmintrin.h>

#include <cmath>
#include <ostream>

#include "Vector3.h"
#include "../util/Logger.h"

inline __m128 Util::loadToReg(const float* q)
{
    return _mm_load_ps(q);
}

inline void Util::saveBack(float* q, __m128 reg)
{
    _mm_store_ps(q, reg);
}

inline __m128 quatMultiply(__m128 a, __m128 b)
{
    __m128 a_wwww = _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 0));
    __m128 a_xxxx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 1, 1, 1));
    __m128 a_yyyy = _mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 2, 2, 2));
    __m128 a_zzzz = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 3, 3, 3));

    __m128 result = _mm_mul_ps(a_wwww, b); // a.w * b

    __m128 temp = _mm_mul_ps(a_xxxx, _mm_shuffle_ps(b, b, _MM_SHUFFLE(1, 0, 3, 2)));
    temp = _mm_mul_ps(temp, _mm_setr_ps(1.0f, 1.0f, -1.0f, 1.0f));
    result = _mm_add_ps(result, temp);

    temp = _mm_mul_ps(a_yyyy, _mm_shuffle_ps(b, b, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm_mul_ps(temp, _mm_setr_ps(1.0f, -1.0f, 1.0f, 1.0f));
    result = _mm_add_ps(result, temp);

    temp = _mm_mul_ps(a_zzzz, _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 2, 1, 0)));
    temp = _mm_mul_ps(temp, _mm_setr_ps(-1.0f, 1.0f, 1.0f, 1.0f));
    result = _mm_add_ps(result, temp);

    return result;
}

//toDo not working
void Util::rotate(float real[4], float dual[4], const float rotation[4])
{
    __m128 q_real = _mm_load_ps(real);
    __m128 q_dual = _mm_load_ps(dual);
    __m128 rot    = _mm_load_ps(rotation);

    q_real = quatMultiply(rot, q_real);
    q_dual = quatMultiply(rot, q_dual);

    _mm_store_ps(real, q_real);
    _mm_store_ps(dual, q_dual);
}

void Util::translate(float real[4], float dual[4], const float translation[3])
{
    const float tx = 2.0f * (dual[1]*real[0] - dual[0]*real[1] - dual[2]*real[3] + dual[3]*real[2]) + translation[0];
    const float ty = 2.0f * (dual[2]*real[0] - dual[0]*real[2] - dual[3]*real[1] + dual[1]*real[3]) + translation[1];
    const float tz = 2.0f * (dual[3] * real[0] - dual[0] * real[3] - dual[1] * real[2] + dual[2] * real[1]) + translation[2];

    dual[0] = -0.5f * (tx * real[1] + ty * real[2] + tz * real[3]);
    dual[1] = 0.5f * (tx * real[0] + ty * real[3] - tz * real[2]);
    dual[2] = 0.5f * (ty * real[0] + tz * real[1] - tx * real[3]);
    dual[3] = 0.5f * (tz * real[0] + tx * real[2] - ty * real[1]);
}

void Util::translateAndRotate(float real[4], float dual[4], const float rotation[4], const float translation[3])
{
    __m128 q_real = loadToReg(real);
    __m128 q_dual = loadToReg(dual);
    __m128 rot = loadToReg(rotation);
    __m128 rot_conj = _mm_mul_ps(rot, _mm_set_ps(-1.0f, -1.0f, -1.0f, 1.0f));

    q_real = quatMultiply(rot, quatMultiply(q_real, rot_conj));
    q_dual = quatMultiply(rot, quatMultiply(q_dual, rot_conj));

    __m128 t_quat = _mm_set_ps(0.0f, translation[2], translation[1], translation[0]);
    __m128 half = _mm_set1_ps(0.5f);
    q_dual = _mm_add_ps(q_dual, _mm_mul_ps(half, quatMultiply(t_quat, q_real)));

    saveBack(real, q_real);
    saveBack(dual, q_dual);
}

void Util::normFull(float* real, float* dual)
{
    __m128 realReg = _mm_loadu_ps(real);
    __m128 realLenSq = _mm_dp_ps(realReg, realReg, 0xFF);

    float lenSq = _mm_cvtss_f32(realLenSq);
    if (lenSq < 1e-8f)
    {
        real[0] = 1.0f;
        real[1] = real[2] = real[3] = 0.0f;
        dual[0] = dual[1] = dual[2] = dual[3] = 0.0f;
        return;
    }

    __m128 invLen = _mm_set1_ps(1.0f / std::sqrt(lenSq));
    realReg = _mm_mul_ps(realReg, invLen);
    _mm_storeu_ps(real, realReg);
    __m128 dualReg = _mm_loadu_ps(dual);
    __m128 dotProd = _mm_dp_ps(realReg, dualReg, 0xFF);
    __m128 correction = _mm_mul_ps(realReg, dotProd);
    dualReg = _mm_sub_ps(dualReg, correction);

    _mm_storeu_ps(dual, dualReg);
}

//toDo scale

void Util::modelMat4ToDualQuat(const Mat4& mat, float dual[4], float real[4], float scale[3])
{
    const float tx = mat(0, 3);
    const float ty = mat(1, 3);
    const float tz = mat(2, 3);

    const float sx = std::sqrt(mat(0, 0) * mat(0, 0) + mat(0, 1) * mat(0, 1) + mat(0, 2) * mat(0, 2));
    const float sy = std::sqrt(mat(1, 0) * mat(1, 0) + mat(1, 1) * mat(1, 1) + mat(1, 2) * mat(1, 2));
    const float sz = std::sqrt(mat(2, 0) * mat(2, 0) + mat(2, 1) * mat(2, 1) + mat(2, 2) * mat(2, 2));

    const float oy = -std::asin(mat(2, 0) / sz);
    float ox;
    float oz;
    if (std::abs(mat(2, 0) / sz) < 0.999f)
    {
        ox = std::atan2(mat(1, 0) / sy, mat(0, 0) / sx);
        oz = std::atan2(-mat(2, 1) / sz, mat(2, 2) / sz);
    }
    else
    {
        ox = 0;
        oz = std::atan2(-mat(0, 1) / sx, mat(1, 1) / sy);
    }

    scale[0] = sx;
    scale[1] = sy;
    scale[2] = sz;

    fromVec3Rot3ToDualQuat(std::array{tx, ty, tz}.data(), std::array{ox, oy, oz}.data(), dual, real);
}

Mat4 Util::dualQuatToModelMat(const float* r, const float* d, const float* sc)
{
    Mat4 mtx;

    const float tx = 2.0f * (-d[0] * r[1] + d[1] * r[0] - d[2] * r[3] + d[3] * r[2]);
    const float ty = 2.0f * (-d[0] * r[2] + d[1] * r[3] + d[2] * r[0] - d[3] * r[1]);
    const float tz = 2.0f * (-d[0] * r[3] - d[1] * r[2] + d[2] * r[1] + d[3] * r[0]);

    const float qx = r[1];
    const float qy = r[2];
    const float qz = r[3];
    const float qw = r[0];

    const float xx = qx * qx;
    const float yy = qy * qy;
    const float zz = qz * qz;
    const float xy = qx * qy;
    const float xz = qx * qz;
    const float yz = qy * qz;
    const float wx = qw * qx;
    const float wy = qw * qy;
    const float wz = qw * qz;

    mtx(0, 0) = (1.0f - 2.0f * yy - 2.0f * zz) * sc[0];
    mtx(0, 1) = (2.0f * xy - 2.0f * wz) * sc[1];
    mtx(0, 2) = (2.0f * xz + 2.0f * wy) * sc[2];
    mtx(0, 3) = tx;

    mtx(1, 0) = (2.0f * xy + 2.0f * wz) * sc[0];
    mtx(1, 1) = (1.0f - 2.0f * xx - 2.0f * zz) * sc[1];
    mtx(1, 2) = (2.0f * yz - 2.0f * wx) * sc[2];
    mtx(1, 3) = ty;

    mtx(2, 0) = (2.0f * xz - 2.0f * wy) * sc[0];
    mtx(2, 1) = (2.0f * yz + 2.0f * wx) * sc[1];
    mtx(2, 2) = (1.0f - 2.0f * xx - 2.0f * yy) * sc[2];
    mtx(2, 3) = tz;

    mtx(3, 0) = 0.0f;
    mtx(3, 1) = 0.0f;
    mtx(3, 2) = 0.0f;
    mtx(3, 3) = 1.0f;

    return mtx;
}

Mat4 Util::makeViewMatrix(const float* dual, const float* real)
{
    Mat4 mat{};

    const float tx = 2.0f * (dual[1]*real[0] - dual[0]*real[1] - dual[2]*real[3] + dual[3]*real[2]);
    const float ty = 2.0f * (dual[2]*real[0] - dual[0]*real[2] - dual[3]*real[1] + dual[1]*real[3]);
    const float tz = 2.0f * (dual[3] * real[0] - dual[0] * real[3] - dual[1] * real[2] + dual[2] * real[1]);

    const float w = real[0];
    const float x = real[1];
    const float y = real[2];
    const float z = real[3];

    const float xx = x * x;
    const float yy = y * y;
    const float zz = z * z;
    const float xy = x * y;
    const float xz = x * z;
    const float yz = y * z;
    const float wx = w * x;
    const float wy = w * y;
    const float wz = w * z;

    float R[3][3];
    R[0][0] = 1.0f - 2.0f * (yy + zz);
    R[0][1] = 2.0f * (xy + wz);
    R[0][2] = 2.0f * (xz - wy);

    R[1][0] = 2.0f * (xy - wz);
    R[1][1] = 1.0f - 2.0f * (xx + zz);
    R[1][2] = 2.0f * (yz + wx);

    R[2][0] = 2.0f * (xz + wy);
    R[2][1] = 2.0f * (yz - wx);
    R[2][2] = 1.0f - 2.0f * (xx + yy);

    mat(0, 0) = R[0][0]; mat(1, 0) = R[0][1]; mat(2, 0) = R[0][2];
    mat(0, 1) = R[1][0]; mat(1, 1) = R[1][1]; mat(2, 1) = R[1][2];
    mat(0, 2) = R[2][0]; mat(1, 2) = R[2][1]; mat(2, 2) = R[2][2];

    mat(0, 3) = -(R[0][0]*tx + R[0][1]*ty + R[0][2]*tz);
    mat(1, 3) = -(R[1][0]*tx + R[1][1]*ty + R[1][2]*tz);
    mat(2, 3) = -(R[2][0]*tx + R[2][1]*ty + R[2][2]*tz);

    std::swap(mat(1, 0), mat(0, 1));
    std::swap(mat(2, 0), mat(0, 2));
    std::swap(mat(2, 1), mat(1, 2));

    return mat;
}

Mat4 Util::makeProjMatrixPerspective(const float fovY, const float aspect, const float zNear, const float zFar)
{
    const float ctan = std::cos(fovY / 2) / std::sin(fovY / 2);
    const float range = zFar - zNear;

    alignas(16) float m[16] = {
        ctan / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, -ctan, 0.0f, 0.0f,
        0.0f, 0.0f, -zFar / range, -1.0f,
        0.0f, 0.0f, -zFar * zNear / range, 0.0f
    };

    return Mat4(m);
}

Mat4 Util::makeProjMatrixOrtographic(const float xmag, const float ymag, const float znear, const float zfar)
{
    const float range = zfar - znear;

    alignas(16) float m[16] = {
        1.0f / xmag, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f / ymag, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f / range, -1.0f,
        0.0f, 0.0f, - znear / range, 0.0f
    };

    return Mat4(m);
}

void Util::fromVec3Rot3ToDualQuat(const float t[3], const float r[3], float dual[4], float real[4])
{
    const float ox = r[0];
    const float oy = r[1];
    const float oz = r[2];

    const float cx = std::cos(ox / 2.0f);
    const float sx = std::sin(ox / 2.0f);
    const float cy = std::cos(oy / 2.0f);
    const float sy = std::sin(oy / 2.0f);
    const float cz = std::cos(oz / 2.0f);
    const float sz = std::sin(oz / 2.0f);

    real[0] = cz * cy * cx + sz * sy * sx;
    real[1] = cz * cy * sx + sz * sy * cx;
    real[2] = -cz * sy * cx + sz * cy * sx;
    real[3] = -cz * sy * sx + sz * cy * cx;

    dual[0] = -0.5f * (t[0] * real[1] + t[1] * real[2] + t[2] * real[3]);
    dual[1] = 0.5f * (t[0] * real[0] + t[1] * real[3] - t[2] * real[2]);
    dual[2] = 0.5f * (t[1] * real[0] + t[2] * real[1] - t[0] * real[3]);
    dual[3] = 0.5f * (t[2] * real[0] + t[0] * real[2] - t[1] * real[1]);
}

void Util::setTranslate(const float translation[3], const float real[4], float dual[4])
{
    __m128 t = _mm_set_ps(0.0f, translation[2], translation[1], translation[0]);
    __m128 q_r = _mm_load_ps(real);

    __m128 half_t = _mm_mul_ps(t, _mm_set1_ps(0.5f));

    __m128 t_wwww = _mm_shuffle_ps(half_t, half_t, _MM_SHUFFLE(3, 3, 3, 3));
    __m128 t_xxxx = _mm_shuffle_ps(half_t, half_t, _MM_SHUFFLE(0, 0, 0, 0));
    __m128 t_yyyy = _mm_shuffle_ps(half_t, half_t, _MM_SHUFFLE(1, 1, 1, 1));
    __m128 t_zzzz = _mm_shuffle_ps(half_t, half_t, _MM_SHUFFLE(2, 2, 2, 2));

    __m128 q_r_zyxw = _mm_shuffle_ps(q_r, q_r, _MM_SHUFFLE(0, 1, 2, 3));

    __m128 result = _mm_mul_ps(t_wwww, q_r); // w * q_r

    __m128 temp = _mm_mul_ps(t_xxxx, _mm_shuffle_ps(q_r_zyxw, q_r_zyxw, _MM_SHUFFLE(3, 0, 1, 2)));
    result = _mm_add_ps(result, temp);

    temp = _mm_mul_ps(t_yyyy, _mm_shuffle_ps(q_r_zyxw, q_r_zyxw, _MM_SHUFFLE(2, 3, 0, 1)));
    result = _mm_add_ps(result, temp);

    temp = _mm_mul_ps(t_zzzz, _mm_shuffle_ps(q_r_zyxw, q_r_zyxw, _MM_SHUFFLE(1, 2, 3, 0)));
    result = _mm_sub_ps(result, temp);

    _mm_store_ps(dual, result);
}
