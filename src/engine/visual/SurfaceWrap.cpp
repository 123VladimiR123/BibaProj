//
// Created by Владимир on 24.06.2025.
//

#include "SurfaceWrap.h"
#include "../util/Logger.h"

SurfaceWrap::SurfaceWrap(const VkInstance *instance, const GLFWwindow *glfWwindow) {
    if (glfwCreateWindowSurface(*instance, const_cast<GLFWwindow *>(glfWwindow), nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Unable to create surface");
    Logger::get() << "Surface created" << std::endl;
}