//
// Created by admin on 8/19/2025.
//

#ifndef MESH_H
#define MESH_H

#include <memory>
#include <vector>

#include "Material.h"
#include "../../maths/Vertex.h"

struct MeshBufferInfo
{
    uint32_t vertexCount{};
    uint32_t indexCount{};

    VkDeviceSize vertexByteOffset{};
    VkDeviceSize indexByteOffset{};

    uint32_t firstIndex{};
    int32_t vertexOffset{};
};

class MeshPrimitive {
public:
    MeshBufferInfo info;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::shared_ptr<Material> material;

    MeshPrimitive() = default;
    MeshPrimitive(MeshPrimitive&) = delete;

    MeshPrimitive(MeshPrimitive&&) noexcept = default;

    MeshPrimitive& operator=(MeshPrimitive&) = delete;
    MeshPrimitive& operator=(MeshPrimitive&&) noexcept = default;
};

#endif //MESH_H
