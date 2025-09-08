//
// Created by admin on 8/30/2025.
//

#include "Renderer.h"

#include <array>
#include <chrono>
#include <memory>

#include "../gameobj/camera/Camera.h"
#include "util/BufferUtil.h"
#include "./../util/Logger.h"
#include "util/UniformBufferObject.h"

Renderer::Renderer(const VkDevice* device, const VkPhysicalDevice* physicalDevice, const VkCommandPool* commandPool,
                   const VkRenderPass* renderPass,
                   const std::vector<VkImageView>& swapchainImageViews, const VkImageView* depthImageView,
                   const VkExtent2D* extent, VkDescriptorSetLayout* uboLayout, uint32_t maxObjs,
                   uint32_t totalMaterials, uint32_t totalMeshes)
    : maxFramesInFlight(swapchainImageViews.size())
{
    createSyncObjects(device);
    createUboResources(device, physicalDevice, maxObjs);
    createDescriptorPool(device, totalMaterials, totalMeshes);
    createDynamicDescriptorSets(device, uboLayout);
    createCommandBuffers(device, commandPool);
    createFramebuffers(device, renderPass, swapchainImageViews, depthImageView, extent);
}

void Renderer::createFramebuffers(const VkDevice* device,
                                  const VkRenderPass* renderPass,
                                  const std::vector<VkImageView>& swapchainImageViews,
                                  const VkImageView* depthImageView,
                                  const VkExtent2D* extent)
{
    framebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); ++i)
    {
        std::array attachments = {
            swapchainImageViews[i],
            *depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent->width;
        framebufferInfo.height = extent->height;
        framebufferInfo.layers = 1;

        framebuffers[i] = std::make_unique<VkFramebuffer>();
        if (vkCreateFramebuffer(*device, &framebufferInfo, nullptr, framebuffers[i].get()) != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer!");
    }
}

void Renderer::createUboResources(const VkDevice* device, const VkPhysicalDevice* physicalDevice,
                                  const size_t maxObjects)
{
    uboBuffers.resize(maxFramesInFlight);
    uboBuffersMemory.resize(maxFramesInFlight);

    const VkDeviceSize bufferSize = sizeof(UniformBufferObject) * maxObjects;

    for (size_t i = 0; i < maxFramesInFlight; i++)
    {
        BufferUtil::createBuffer(
            *device,
            *physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            uboBuffers[i],
            uboBuffersMemory[i]
        );
    }
}

void Renderer::createCommandBuffers(const VkDevice* device, const VkCommandPool* commandPool)
{
    commandBuffers.resize(maxFramesInFlight);
    std::vector<VkCommandBuffer> tempCommandBuffers(maxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(maxFramesInFlight);

    if (vkAllocateCommandBuffers(*device, &allocInfo, tempCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");

    for (size_t i = 0; i < maxFramesInFlight; i++)
        commandBuffers[i] = tempCommandBuffers[i];
}

void Renderer::createSyncObjects(const VkDevice* device)
{
    imageAvailableSemaphores.resize(maxFramesInFlight);
    renderFinishedSemaphores.resize(maxFramesInFlight);
    inFlightFences.resize(maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < maxFramesInFlight; i++)
    {
        imageAvailableSemaphores[i] = std::make_unique<VkSemaphore>();
        renderFinishedSemaphores[i] = std::make_unique<VkSemaphore>();
        inFlightFences[i] = std::make_unique<VkFence>();

        if (vkCreateSemaphore(*device, &semaphoreInfo, nullptr, imageAvailableSemaphores[i].get()) != VK_SUCCESS ||
            vkCreateSemaphore(*device, &semaphoreInfo, nullptr, renderFinishedSemaphores[i].get()) != VK_SUCCESS ||
            vkCreateFence(*device, &fenceInfo, nullptr, inFlightFences[i].get()) != VK_SUCCESS)
            throw std::runtime_error("Failed to create synchronization objects for a frame");
    }
}

void Renderer::createDescriptorPool(const VkDevice* device, const size_t matCount, const size_t primitivesCount)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(matCount);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    poolInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight + matCount);

    VkDescriptorPool rawDescriptorPool;
    if (vkCreateDescriptorPool(*device, &poolInfo, nullptr, &rawDescriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool");
    descriptorPool = std::make_unique<VkDescriptorPool>(rawDescriptorPool);
}

void Renderer::createDynamicDescriptorSets(const VkDevice* device, VkDescriptorSetLayout* uboLayout)
{
    descriptorSets.resize(maxFramesInFlight);

    for (size_t i = 0; i < maxFramesInFlight; ++i)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = *descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = uboLayout;

        if (vkAllocateDescriptorSets(*device, &allocInfo, &descriptorSets[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate dynamic descriptor set");

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uboBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet uboWrite{};
        uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrite.dstSet = descriptorSets[i];
        uboWrite.dstBinding = 0;
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(*device, 1, &uboWrite, 0, nullptr);
    }
}

void Renderer::updateUniformBuffer(const uint32_t currentFrame, const VkDevice* device, Camera* camera) const
{
    UniformBufferObject ubo{};

    ubo.view = camera->getViewMat();
    ubo.proj = camera->getProjMat();
    ubo.proj(1, 1) *= -1;

    void* data;
    vkMapMemory(*device, uboBuffersMemory[currentFrame], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(*device, uboBuffersMemory[currentFrame]);
}

void printMat4(Mat4* mat)
{
    Logger::get() << "[ ";
    for (float i : mat->data)
    {
        Logger::get() << i << " ";
    }
    Logger::get() << "]" << std::endl;
}

VkResult Renderer::renderFrame(Scene* scene, const VkDevice* device, const VkQueue* graphicsQueue,
                           const VkQueue* presentQueue,
                           const VkSwapchainKHR* swapChain, const VkRenderPass* renderPass, const size_t currentFrame,
                           const VkExtent2D* extent, const VkBuffer* vertexBuffer, const VkBuffer* indexBuffer)
{
    vkWaitForFences(*device, 1, inFlightFences[currentFrame].get(), VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    const VkResult acquireImageRes = vkAcquireNextImageKHR(*device, *swapChain, UINT64_MAX, *imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE,
                          &imageIndex);
    if (acquireImageRes != VK_SUCCESS)
        return acquireImageRes;

    vkResetFences(*device, 1, inFlightFences[currentFrame].get());

    alignas(16) UniformBufferObject ubo{};
    ubo.view = scene->activeCamera->getViewMat();
    ubo.proj = scene->activeCamera->getProjMat();

    void* mappedData;
    vkMapMemory(*device, uboBuffersMemory[currentFrame], 0, sizeof(UniformBufferObject), 0, &mappedData);
    memcpy(mappedData, &ubo, sizeof(UniformBufferObject));
    vkUnmapMemory(*device, uboBuffersMemory[currentFrame]);

    VkCommandBuffer commandBuffer = commandBuffers[currentFrame];
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = *renderPass;
    renderPassInfo.framebuffer = *framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = *extent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.5f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (const auto& object : scene->gameObjsFlat)
    {
        Mat4 model = object->getModelMatrix();

        for (const auto& mesh : object->primitives)
        {
            if (mesh->info.indexCount == 0 || mesh->vertices.empty())
                throw std::runtime_error("Why???");

            const auto& currentPipeline = mesh->material->pipeline;
            const auto& currentPipelineLayout = mesh->material->pipelineLayout;

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);
            std::array setsToBind = {descriptorSets[currentFrame], *mesh->material->descriptor};

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipelineLayout,
                                    0, setsToBind.size(), setsToBind.data(),
                                    0, nullptr);

            vkCmdPushConstants(
                commandBuffer,
                *currentPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(Mat4),
                &model
            );
            vkCmdPushConstants(
                commandBuffer,
                *currentPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                sizeof(Mat4),
                sizeof(Vector4),
                &mesh->material->albedoColor
            );
            VkBuffer vertexBuffers[] = {*vertexBuffer};
            VkDeviceSize vertexOffsets[] = {mesh->info.vertexByteOffset};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, vertexOffsets );

            vkCmdBindIndexBuffer(commandBuffer, *indexBuffer, mesh->info.indexByteOffset, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(
                commandBuffer,
                mesh->info.indexCount,
                1,
                0,
                0,
                0
            );
        }
    }

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");

    //submitting
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    const VkSemaphore waitSemaphores[] = {*imageAvailableSemaphores[currentFrame]};
    const VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    const VkSemaphore signalSemaphores[] = {*renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(*graphicsQueue, 1, &submitInfo, *inFlightFences[currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit draw command buffer");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    const VkSwapchainKHR swapChains[] = {*swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(*presentQueue, &presentInfo);

    return VK_SUCCESS;
}

VkDescriptorSetLayout* Renderer::makeUboLayout(VkDevice* device)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    const auto result = new VkDescriptorSetLayout();
    if (vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, result) != VK_SUCCESS)
        throw std::runtime_error("Failed to create ubo descriptor set layout");
    return result;
}

void Renderer::cleanup(const VkDevice* device, const VkCommandPool* pool)
{
    vkDeviceWaitIdle(*device);
    for (const auto& fence: inFlightFences)
        vkDestroyFence(*device, *fence, nullptr);
    for (const auto& semaphore : imageAvailableSemaphores)
        vkDestroySemaphore(*device, *semaphore, nullptr);
    for (const auto& semaphore : renderFinishedSemaphores)
        vkDestroySemaphore(*device, *semaphore, nullptr);
    vkFreeCommandBuffers(*device, *pool, commandBuffers.size(), commandBuffers.data());
    for (const auto& buffer : framebuffers)
        vkDestroyFramebuffer(*device, *buffer, nullptr);
    vkDestroyDescriptorPool(*device, *descriptorPool, nullptr);
    for (const auto& uboBuffer: uboBuffers)
        vkDestroyBuffer(*device, uboBuffer, nullptr);
    for (const auto& mem: uboBuffersMemory)
        vkFreeMemory(*device, mem, nullptr);
}
