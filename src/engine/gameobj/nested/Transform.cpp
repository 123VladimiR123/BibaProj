#include "Transform.h"

#include "../../visual/util/BufferUtil.h"

Transform::Transform()
{
    this->dirty = false;
    this->mtrx = Mat4{};
}

Transform::Transform(Mat4&& mtrx)
{
    this->dirty = false;
    this->mtrx = mtrx;
}

Transform::Transform(Mat4& mtrx)
{
    this->dirty = false;
    this->mtrx = mtrx;
}

Transform::Transform(DualQuat&& quat)
{
    this->quat = quat;
    this->dirty = true;
}

Transform::Transform(DualQuat quat)
{
    this->quat = quat;
    this->dirty = true;
}

std::optional<DualQuat>& Transform::getQuat()
{
    dirty = true;
    return quat;
}


Mat4& Transform::getMatrix()
{
    if (!this->dirty) return this->mtrx;
    this->quat->normFull();
    const std::array<float, 3> scaleArr = scale.has_value()
                                              ? std::array{scale.value().x, scale.value().y, scale.value().z}
                                              : std::array{1.0f, 1.0f, 1.0f};
    this->mtrx = this->quat->getModelMat4(scaleArr.data());
    this->dirty = false;
    return this->mtrx;
}

void Transform::moveAndRotate(const Quat& quatIn, const float translation[3])
{
    if (!this->quat) return;
    this->quat->translateAndRotate(quatIn, translation);
    this->dirty = true;
}

void Transform::move(const float translation[3])
{
    if (!this->quat.has_value()) return;
    this->quat->translate(translation);
    this->dirty = true;
}

void Transform::rotate(const Quat& quatIn)
{
    if (!this->quat) return;
    this->quat->rotate(quatIn);
    this->dirty = true;
}

void Transform::emplaceFromMat4()
{
    float realArr[4]{};
    float dualArr[4]{};
    float scaleArr[3]{};
    Util::modelMat4ToDualQuat(mtrx, dualArr, realArr, scaleArr);

    Quat realQuat{realArr};
    quat.emplace(std::move(realQuat));
    quat.value().getDual()[0] = dualArr[0];
    quat.value().getDual()[1] = dualArr[1];
    quat.value().getDual()[2] = dualArr[2];
    quat.value().getDual()[3] = dualArr[3];

    scale.emplace(scaleArr[0], scaleArr[1], scaleArr[2]);
    this->dirty = true;
}