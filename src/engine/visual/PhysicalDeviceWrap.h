//
// Created by Владимир on 08.08.2025.
//

#ifndef BIBAPROJ_PHYSICALDEVICEWRAP_H
#define BIBAPROJ_PHYSICALDEVICEWRAP_H


#include <vulkan/vulkan_core.h>

class PhysicalDeviceWrap {
    VkPhysicalDevice device = VK_NULL_HANDLE;

public:
    VkPhysicalDeviceFeatures features{};

    PhysicalDeviceWrap(const PhysicalDeviceWrap &) = delete;

    PhysicalDeviceWrap &operator=(const PhysicalDeviceWrap &) = delete;

    explicit PhysicalDeviceWrap(VkInstance *);

    VkPhysicalDevice *get() { return &device; };
};


#endif //BIBAPROJ_PHYSICALDEVICEWRAP_H
