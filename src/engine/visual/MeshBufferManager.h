//
// Created by admin on 8/29/2025.
//

#ifndef MESHBUFFERMANAGER_H
#define MESHBUFFERMANAGER_H
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "./../gameobj/nested/MeshPrimitive.h"

class MeshBufferManager
{
    std::unique_ptr<VkBuffer, std::function<void(VkBuffer*)>> vertexBuffer;
    std::unique_ptr<VkDeviceMemory, std::function<void(VkDeviceMemory*)>> vertexMemory;

    std::unique_ptr<VkBuffer, std::function<void(VkBuffer*)>> indexBuffer;
    std::unique_ptr<VkDeviceMemory, std::function<void(VkDeviceMemory*)>> indexMemory;

public:
    MeshBufferManager(const std::vector<std::shared_ptr<MeshPrimitive>>& allMeshes, VkDevice* device, VkPhysicalDevice* physicalDevice,
                      VkCommandPool* commandPool, VkQueue* transferQueue, const std::function<void(
                          VkDevice, VkPhysicalDevice, VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags,
                          VkBuffer&, VkDeviceMemory&)>& createBuffer,
                      const std::function<void(VkDevice, VkCommandPool, VkQueue, VkBuffer,
                                               VkBuffer, VkDeviceSize)>&);

    MeshBufferManager(MeshBufferManager&) = delete;
    MeshBufferManager(MeshBufferManager&&) noexcept = default;

    MeshBufferManager& operator=(MeshBufferManager&) = delete;
    MeshBufferManager& operator=(MeshBufferManager&&) noexcept = default;


    [[nodiscard]] const VkBuffer* getVertexBuffer() const { return vertexBuffer.get(); }
    [[nodiscard]] const VkDeviceMemory* getVertexMemory() const { return vertexMemory.get(); }

    [[nodiscard]] const VkBuffer* getIndexBuffer() const { return indexBuffer.get(); }
    [[nodiscard]] const VkDeviceMemory* getIndexMemory() const { return indexMemory.get(); }
};

#endif //MESHBUFFERMANAGER_H
