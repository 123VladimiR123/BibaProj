//
// Created by admin on 8/30/2025.
//

#ifndef DEPTHBUFFER_H
#define DEPTHBUFFER_H
#include <functional>
#include <memory>
#include <vulkan/vulkan_core.h>


class DepthBuffer {
public:
    std::unique_ptr<VkImage, std::function<void(VkImage*)>> image;
    std::unique_ptr<VkDeviceMemory, std::function<void(VkDeviceMemory*)>> imageMemory;
    std::unique_ptr<VkImageView, std::function<void(VkImageView*)>> imageView;

    DepthBuffer(VkDevice device, VkPhysicalDevice *physicalDevice, VkExtent2D *swapchainExtent);
};


#endif //DEPTHBUFFER_H
