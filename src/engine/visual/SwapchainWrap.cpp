//
// Created by Владимир on 29.06.2025.
//

#include <iostream>
#include <algorithm>
#include "SwapchainWrap.h"

#include <limits>

#include "../util/Logger.h"

SwapchainWrap::SwapchainWrap(VkSurfaceKHR* surface, VkDevice* device, VkPhysicalDevice* physicalDevice,
                             QueueFamilyIndices* indices, GLFWwindow* window)
{

    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwWaitEvents();
        glfwGetFramebufferSize(window, &width, &height);
    }

    auto [capabilities, formats, presentModes] = checkSupport(physicalDevice, surface);

    chooseSwapSurfaceFormat(formats);
    const VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
    extent = chooseSwapExtent(capabilities, window);

    uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = *surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (indices->graphicsFamily != indices->presentFamily)
    {
        const uint32_t families[] = {indices->graphicsFamily.value(), indices->presentFamily.value()};

        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = families;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = swapchain;

    if (vkCreateSwapchainKHR(*device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
        throw std::runtime_error("Unable to create swapchain!");
    Logger::get() << "SwapChain created" << std::endl;
}

SwapChainSupportDetails SwapchainWrap::checkSupport(VkPhysicalDevice* physicalDevice, VkSurfaceKHR* surface)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*physicalDevice, *surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice, *surface, &formatCount, nullptr);

    if (formatCount == 0)
        throw std::runtime_error("Device does not support any of formats");
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice, *surface, &formatCount, details.formats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*physicalDevice, *surface, &presentModeCount, nullptr);

    if (presentModeCount == 0)
        throw std::runtime_error("Device does not support any of presentation modes");
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(*physicalDevice, *surface, &presentModeCount,
                                              details.presentModes.data());

    return details;
}

void SwapchainWrap::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            format = availableFormat;
            return;
        }
    }

    format = availableFormats[0];
}

VkPresentModeKHR SwapchainWrap::chooseSwapPresentMode(std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapchainWrap::chooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (std::numeric_limits<uint32_t>::max() != capabilities.currentExtent.width)
    {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);

    return actualExtent;
}
