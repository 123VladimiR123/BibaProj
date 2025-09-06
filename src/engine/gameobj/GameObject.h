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

    bool visible = true;
    uint32_t nodeIdx{};

    //todo make mtrx getter by multiply transform.getMatrix() and parent.getMatrix()

    Mat4 getModelMatrix() const
    {
        if (parent != nullptr)
            return parent->getModelMatrix() * transform->getMatrix();
        return transform->getMatrix();
    }

    GameObject() = default;
    GameObject(GameObject&) = delete;

    GameObject(GameObject&&) noexcept = default;

    GameObject& operator=(GameObject&) = delete;
    GameObject& operator=(GameObject&&) noexcept = default;
};

#endif //GAMEOBJECT_H
