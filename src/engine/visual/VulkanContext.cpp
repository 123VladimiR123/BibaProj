//
// Created by Владимир on 08.08.2025.
//

#include "VulkanContext.h"

std::unique_ptr<InstanceWrap> VulkanContext::pInstance{nullptr};
std::unique_ptr<DeviceWrap> VulkanContext::pDevice{nullptr};
std::unique_ptr<PhysicalDeviceWrap> VulkanContext::pPhysicalDevice{nullptr};
std::unique_ptr<SwapchainWrap> VulkanContext::pSwapchain{nullptr};
std::unique_ptr<SurfaceWrap> VulkanContext::pSurface{nullptr};
std::unique_ptr<WindowWrap> VulkanContext::pWindow{nullptr};
std::unique_ptr<QueueWrap> VulkanContext::pQueue{nullptr};
std::unique_ptr<SwapchainResourceManager> VulkanContext::pSwapchainManager{nullptr};
std::unique_ptr<ShaderManager> VulkanContext::pShader{nullptr};
std::unique_ptr<CommandPoolWrap> VulkanContext::pCommandPool{nullptr};
std::unique_ptr<DepthBuffer> VulkanContext::pDepth{nullptr};