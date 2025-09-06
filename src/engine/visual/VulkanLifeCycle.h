#pragma once
#include "Renderer.h"
#include "GLFW/glfw3.h"

class VulkanLifeCycle
{
    uint32_t frameCounter{};

    std::unique_ptr<Renderer> renderer;
public:
    VulkanLifeCycle();
    VulkanLifeCycle(const VulkanLifeCycle&) = delete;
    VulkanLifeCycle(const VulkanLifeCycle&&) = delete;

    VulkanLifeCycle& operator=(const VulkanLifeCycle&) = delete;
    VulkanLifeCycle& operator=(const VulkanLifeCycle&&) = delete;

    static void showExtensions();

    void fillSceneWithVulkan(Scene *scene);
    void render(Scene* scene);
    GLFWwindow *getWindow();
    void closeScene(Scene* scene);

    ~VulkanLifeCycle();
};
