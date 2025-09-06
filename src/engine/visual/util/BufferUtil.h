//
// Created by admin on 8/29/2025.
//

#ifndef BUFFERUTIL_H
#define BUFFERUTIL_H
#include <vector>
#include <vulkan/vulkan_core.h>


class BufferUtil
{
public:
    static void createBuffer(VkDevice, VkPhysicalDevice, VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags,
                             VkBuffer&, VkDeviceMemory&);
    static void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue transferQueue, VkBuffer srcBuffer,
                           VkBuffer dstBuffer, VkDeviceSize size);
    static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                                   VkMemoryPropertyFlags properties);

    static VkFormat findSupportedFormat(const VkPhysicalDevice* physicalDevice,
                                                    const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                                    VkFormatFeatureFlags features);
};


#endif //BUFFERUTIL_H
