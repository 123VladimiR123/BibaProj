//
// Created by ???????? on 09.08.2025.
//

#include <iostream>
#include "SwapchainResourceManager.h"
#include "../util/Logger.h"

void
SwapchainResourceManager::makeSwapchainImages(VkSwapchainKHR* swapchain, VkDevice* device, VkSurfaceFormatKHR* format)
{
    uint32_t imageCount{};

    vkGetSwapchainImagesKHR(*device, *swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    swapchainViews.resize(imageCount);
    vkGetSwapchainImagesKHR(*device, *swapchain, &imageCount, images.data());

    Logger::get() << std::endl << "SwapChain's VkImage created. Total: " << imageCount << std::endl;

    for (size_t i{}; i < images.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format->format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(*device, &createInfo, nullptr, &swapchainViews[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swapchain's image view");

        Logger::get() << "Swapchain's view created: " << i + 1 << " of " << imageCount << std::endl;
    }
}

void SwapchainResourceManager::destroySwapchainImages(VkDevice* device)
{
    for (VkImageView& view : swapchainViews)
        vkDestroyImageView(*device, view, nullptr);
}
