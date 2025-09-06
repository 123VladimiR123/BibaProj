//
// Created by Владимир on 24.06.2025.
//

#include <vulkan/vulkan_core.h>
#include <memory>
#include "GLFW/glfw3.h"
#include "DeviceWrap.h"

class SurfaceWrap {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
public:
    SurfaceWrap(const SurfaceWrap &) = delete;

    SurfaceWrap &operator=(const SurfaceWrap &) = delete;

    SurfaceWrap(const VkInstance *, const GLFWwindow *);

    VkSurfaceKHR *get() { return &surface; }
};