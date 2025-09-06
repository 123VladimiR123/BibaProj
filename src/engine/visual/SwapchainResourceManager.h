//
// Created by ???????? on 09.08.2025.
//

#ifndef BIBAPROJ_SWAPCHAINRESOURCEMANAGER_H
#define BIBAPROJ_SWAPCHAINRESOURCEMANAGER_H

#include <vector>
#include <vulkan/vulkan_core.h>

class SwapchainResourceManager {
    std::vector<VkImage> images;
    std::vector<VkImageView> swapchainViews;
public:
    SwapchainResourceManager(const SwapchainResourceManager &) = delete;

    SwapchainResourceManager &operator=(const SwapchainResourceManager &) = delete;

    SwapchainResourceManager() = default;

    void makeSwapchainImages(VkSwapchainKHR *, VkDevice *, VkSurfaceFormatKHR *);

    void destroySwapchainImages(VkDevice *);

    std::vector<VkImageView>& getSwapchainImages() { return swapchainViews; }
};


#endif //BIBAPROJ_SWAPCHAINRESOURCEMANAGER_H
