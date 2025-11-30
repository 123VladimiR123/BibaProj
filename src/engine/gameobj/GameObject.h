//
// Created by admin on 8/19/2025.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <memory>

#include "nested/MeshPrimitive.h"
#include "nested/Transform.h"


class GameObject {
public:
    std::unique_ptr<Transform> transform;

    std::vector<std::shared_ptr<MeshPrimitive>> primitives;

    std::shared_ptr<GameObject> parent;
    std::vector<std::shared_ptr<GameObject>> children;

    uint_fast8_t markable = 0;

    bool visible = true;
    uint32_t nodeIdx{};

    Mat4 getModelMatrix() const //todo bottleneck?
    {
        if (parent != nullptr)
            return Util::multiply(parent->getModelMatrix(), transform->getMatrix());
        return transform->getMatrix();
    }

    GameObject() = default;
    GameObject(const GameObject&);

    GameObject(GameObject&&) noexcept = default;

    GameObject& operator=(GameObject&) = default;
    GameObject& operator=(GameObject&&) noexcept = default;
};

inline GameObject::GameObject(const GameObject& other)
{
    if (other.transform) {
        transform = std::make_unique<Transform>(*other.transform);
    } else {
        transform = nullptr;
    }

    primitives = other.primitives;

    parent = other.parent;
    children = other.children;

    markable = other.markable;
    visible  = other.visible;
    nodeIdx  = other.nodeIdx;
}

#endif //GAMEOBJECT_H
