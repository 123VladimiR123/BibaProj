#pragma once

#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <iomanip>
#include <vector>
#include "../util/Cfg.h"
#include "util/QueueFamilyIndices.h"

class DeviceWrap {
    VkDevice device = VK_NULL_HANDLE;
    std::unique_ptr<QueueFamilyIndices> indices;
public:
    DeviceWrap(const DeviceWrap &) = delete;

    DeviceWrap &operator=(const DeviceWrap &) = delete;

    explicit DeviceWrap(VkSurfaceKHR *, VkPhysicalDevice *, VkPhysicalDeviceFeatures &);

    void defineQueueIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    inline VkDevice *get() { return &device; };

    inline QueueFamilyIndices *getIndices() { return indices.get(); };
};