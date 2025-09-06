//
// Created by Владимир on 08.08.2025.
//

#include <stdexcept>
#include <iostream>
#include <vulkan/vulkan_core.h>
#include "InstanceWrap.h"
#include "./../../util/Logger.h"
#include "GLFW/glfw3.h"

InstanceWrap::InstanceWrap() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    Logger::get() << std::endl << "GLFW initiated successfully" << std::endl;

    VkInstanceCreateInfo createInfo{};

    uint32_t counter{};
    const char** extensions = glfwGetRequiredInstanceExtensions(&counter);

#ifdef ENGINE_DEBUG_MODE_VALIDATION
    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = layers;
#endif
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.enabledExtensionCount = counter;
    createInfo.ppEnabledExtensionNames = extensions;

    if (const VkResult res = vkCreateInstance(&createInfo, nullptr, &instance); res == VK_SUCCESS)
        Logger::get() << "Instance created successfully" << std::endl;
    else
        throw std::runtime_error("Error was thrown while attempting to create instance");
}
