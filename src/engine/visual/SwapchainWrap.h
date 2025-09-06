//
// Created by Владимир on 29.06.2025.
//

#ifndef BIBAPROJ_SWAPCHAINWRAP_H
#define BIBAPROJ_SWAPCHAINWRAP_H

#include <vulkan/vulkan.h>
#include <vector>
#include "GLFW/glfw3.h"
#include "util/QueueFamilyIndices.h"

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class SwapchainWrap
{
    VkSwapchainKHR swapchain{};
    VkSurfaceFormatKHR format{};
    VkExtent2D extent{};

    static SwapChainSupportDetails checkSupport(VkPhysicalDevice*, VkSurfaceKHR*);

public:
    SwapchainWrap(const SwapchainWrap&) = delete;

    SwapchainWrap& operator=(const SwapchainWrap&) = delete;

    explicit SwapchainWrap(VkSurfaceKHR*, VkDevice*, VkPhysicalDevice*, QueueFamilyIndices*, GLFWwindow*);

    static VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR>&);

    void chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>&);

    VkSwapchainKHR* get() { return &this->swapchain; }

    VkSurfaceFormatKHR* getFormat() { return &this->format; }

    VkExtent2D* getExtent() { return &this->extent; }

    static VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow*);
};


#endif //BIBAPROJ_SWAPCHAINWRAP_H
