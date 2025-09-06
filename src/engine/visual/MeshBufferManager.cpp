//
// Created by admin on 8/29/2025.
//

#include "MeshBufferManager.h"

#include "../util/Logger.h"

MeshBufferManager::MeshBufferManager(const std::vector<std::shared_ptr<MeshPrimitive>>& allMeshes, VkDevice* device,
                                     VkPhysicalDevice* physicalDevice, VkCommandPool* commandPool,
                                     VkQueue* transferQueue,
                                     const std::function<void(
                                         VkDevice, VkPhysicalDevice, VkDeviceSize, VkBufferUsageFlags,
                                         VkMemoryPropertyFlags,
                                         VkBuffer&, VkDeviceMemory&)>& createBuffer,
                                     const std::function<void(VkDevice, VkCommandPool, VkQueue, VkBuffer,
                                                              VkBuffer, VkDeviceSize)>& copyBuffer)
{
    const auto deleteBuffer = [device](const VkBuffer* buffer)
    {
        vkDestroyBuffer(*device, *buffer, nullptr);
    };

    const auto deleteMemory = [device](const VkDeviceMemory* memory)
    {
        vkFreeMemory(*device, *memory, nullptr);
    };

    VkDeviceSize totalVertexSize = 0;
    VkDeviceSize totalIndexSize = 0;
    for (const auto& mesh : allMeshes)
    {
        totalVertexSize += sizeof(Vertex) * mesh->vertices.size();
        totalIndexSize += sizeof(uint32_t) * mesh->indices.size();
    }

    // temporary
    VkBuffer vertexBufferTemp;
    VkDeviceMemory vertexMemTemp;
    createBuffer(*device, *physicalDevice, totalVertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferTemp,
                 vertexMemTemp);

    VkBuffer indexBufferTemp;
    VkDeviceMemory indexMemTemp;
    createBuffer(*device, *physicalDevice, totalIndexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferTemp,
                 indexMemTemp);
    // end temporary

    vertexBuffer = std::unique_ptr<VkBuffer, std::function<void(VkBuffer*)>>(new VkBuffer(), deleteBuffer);
    vertexMemory = std::unique_ptr<VkDeviceMemory, std::function<void(VkDeviceMemory*)>>(
        new VkDeviceMemory(), deleteMemory);
    indexBuffer = std::unique_ptr<VkBuffer, std::function<void(VkBuffer*)>>(new VkBuffer(), deleteBuffer);
    indexMemory = std::unique_ptr<VkDeviceMemory, std::function<void(VkDeviceMemory*)>>(
        new VkDeviceMemory(), deleteMemory);
    createBuffer(*device, *physicalDevice, totalVertexSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *vertexBuffer, *vertexMemory);
    createBuffer(*device, *physicalDevice, totalIndexSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *indexBuffer, *indexMemory);

    void* vertexData;
    vkMapMemory(*device, vertexMemTemp, 0, totalVertexSize, 0, &vertexData);

    void* indexData;
    vkMapMemory(*device, indexMemTemp, 0, totalIndexSize, 0, &indexData);

    VkDeviceSize currentVertexOffset = 0;
    VkDeviceSize currentIndexOffset = 0;

    uint32_t currentVertexIndexOffset = 0;
    uint32_t currentIndexIndexOffset = 0;
    for (const auto& mesh: allMeshes) {
        memcpy(static_cast<char*>(vertexData) + currentVertexOffset, mesh->vertices.data(),
               sizeof(Vertex) * mesh->vertices.size());
        memcpy(static_cast<char*>(indexData) + currentIndexOffset, mesh->indices.data(),
               sizeof(uint32_t) * mesh->indices.size());

        MeshBufferInfo info{};
        info.vertexCount = static_cast<uint32_t>(mesh->vertices.size());
        info.indexCount = static_cast<uint32_t>(mesh->indices.size());

        info.vertexByteOffset = currentVertexOffset;
        info.indexByteOffset = currentIndexOffset;

        info.firstIndex = currentIndexIndexOffset;
        info.vertexOffset = static_cast<int32_t>(currentVertexIndexOffset);

        mesh->info = info;

        currentVertexOffset += sizeof(Vertex) * mesh->vertices.size();
        currentIndexOffset += sizeof(uint32_t) * mesh->indices.size();

        currentVertexIndexOffset += static_cast<uint32_t>(mesh->vertices.size());
        currentIndexIndexOffset += static_cast<uint32_t>(mesh->indices.size());
    }
    vkUnmapMemory(*device, vertexMemTemp);
    vkUnmapMemory(*device, indexMemTemp);

    copyBuffer(*device, *commandPool, *transferQueue, vertexBufferTemp, *vertexBuffer, totalVertexSize);
    copyBuffer(*device, *commandPool, *transferQueue, indexBufferTemp, *indexBuffer, totalIndexSize);

    deleteBuffer(&vertexBufferTemp);
    deleteMemory(&vertexMemTemp);
    deleteBuffer(&indexBufferTemp);
    deleteMemory(&indexMemTemp);

    Logger::get() << "Central mesh buffer manager created for scene" << std::endl;
}
