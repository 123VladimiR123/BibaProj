//
// Created by admin on 8/30/2025.
//

#include "DepthBuffer.h"

#include "./../util/Logger.h"
#include "util/BufferUtil.h"

DepthBuffer::DepthBuffer(VkDevice device, VkPhysicalDevice* physicalDevice, VkExtent2D* swapchainExtent)
{
    VkFormat depthFormat = BufferUtil::findSupportedFormat(
        physicalDevice,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapchainExtent->width;
    imageInfo.extent.height = swapchainExtent->height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    image = std::unique_ptr<VkImage, std::function<void(VkImage*)>>(new VkImage(), [device](const VkImage* img)
    {
        vkDestroyImage(device, *img, nullptr);
    });
    if (vkCreateImage(device, &imageInfo, nullptr, image.get()) != VK_SUCCESS)
        throw std::runtime_error("Failed to create depth image");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = BufferUtil::findMemoryType(*physicalDevice, memRequirements.memoryTypeBits,
                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    imageMemory = std::unique_ptr<VkDeviceMemory, std::function<void(VkDeviceMemory*)>>(
        new VkDeviceMemory(), [device](const VkDeviceMemory* mem)
        {
            vkFreeMemory(device, *mem, nullptr);
        });
    if (vkAllocateMemory(device, &allocInfo, nullptr, imageMemory.get()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate depth image memory!");


    vkBindImageMemory(device, *image, *imageMemory, 0);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = *image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    imageView = std::unique_ptr<VkImageView, std::function<void(VkImageView*)>>(
        new VkImageView(), [device](VkImageView* view)
        {
            vkDestroyImageView(device, *view, nullptr);
        });
    if (vkCreateImageView(device, &viewInfo, nullptr, imageView.get()) != VK_SUCCESS)
        throw std::runtime_error("Failed to create depth image view!");

    Logger::get() << "Depth buffer created" << std::endl;
}
