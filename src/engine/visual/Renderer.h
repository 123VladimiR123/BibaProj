//
// Created by admin on 8/30/2025.
//

#ifndef RENDERER_H
#define RENDERER_H
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "../scene/Scene.h"
#include "GLFW/glfw3.h"

class Renderer
{
    std::vector<VkBuffer> uboBuffers;
    std::vector<VkDeviceMemory> uboBuffersMemory;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<std::unique_ptr<VkSemaphore>> imageAvailableSemaphores;
    std::vector<std::unique_ptr<VkSemaphore>> renderFinishedSemaphores;
    std::vector<std::unique_ptr<VkFence>> inFlightFences;
    std::vector<std::unique_ptr<VkFramebuffer>> framebuffers;

    std::unique_ptr<VkDescriptorPool> descriptorPool;

    size_t maxFramesInFlight;

    void createUboResources(const VkDevice* device, const VkPhysicalDevice* physicalDevice, size_t maxObjects);
    void createCommandBuffers(const VkDevice* device, const VkCommandPool* commandPool);
    void createSyncObjects(const VkDevice* device);
    void createDescriptorPool(const VkDevice* device, size_t matCount, size_t primitivesCount);
    void createDynamicDescriptorSets(const VkDevice* device, VkDescriptorSetLayout* uboLayout);
    void updateUniformBuffer(uint32_t currentFrame, const VkDevice* device, Camera* camera) const;
    void createFramebuffers(const VkDevice* device,
                            const VkRenderPass* renderPass,
                            const std::vector<VkImageView>& swapchainImageViews,
                            const VkImageView* depthImageView,
                            const VkExtent2D* extent);

public:
    Renderer(const VkDevice* device, const VkPhysicalDevice* physicalDevice, const VkCommandPool* commandPool,
             const VkRenderPass* renderPass,
             const std::vector<VkImageView>& swapchainImageViews, const VkImageView* depthImageView,
             const VkExtent2D* extent, VkDescriptorSetLayout* uboLayout, uint32_t maxObjs, uint32_t totalMaterials, uint32_t totalMeshes);
    Renderer(Renderer&) = delete;

    Renderer(Renderer&&) noexcept = default;

    Renderer& operator=(Renderer&) = delete;
    Renderer& operator=(Renderer&&) noexcept = default;

    VkResult renderFrame(Scene* scene, const VkDevice* device, const VkQueue* graphicsQueue,
                         const VkQueue* presentQueue,
                         const VkSwapchainKHR* swapChain, const VkRenderPass* renderPass, const size_t currentFrame,
                         const VkExtent2D* extent, const VkBuffer* vertexBuffer, const VkBuffer* indexBuffer, GLFWwindow* window);
    void cleanup(const VkDevice* device, const VkCommandPool* pool);

    static VkDescriptorSetLayout* makeUboLayout(VkDevice *device);
};

#endif //RENDERER_H
