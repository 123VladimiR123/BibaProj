//
// Created by ???????? on 08.08.2025.
//
#pragma once

#include <memory>

#include "CommandPoolWrap.h"
#include "DeviceWrap.h"
#include "SwapchainWrap.h"
#include "SurfaceWrap.h"
#include "InstanceWrap.h"
#include "PhysicalDeviceWrap.h"
#include "WindowWrap.h"
#include "QueueWrap.h"
#include "DepthBuffer.h"
#include "SwapchainResourceManager.h"
#include "ShaderManager.h"

class VulkanContext
{
    static std::unique_ptr<InstanceWrap> pInstance;
    static std::unique_ptr<DeviceWrap> pDevice;
    static std::unique_ptr<QueueWrap> pQueue;
    static std::unique_ptr<PhysicalDeviceWrap> pPhysicalDevice;
    static std::unique_ptr<SwapchainWrap> pSwapchain;
    static std::unique_ptr<SurfaceWrap> pSurface;
    static std::unique_ptr<WindowWrap> pWindow;
    static std::unique_ptr<SwapchainResourceManager> pSwapchainManager;
    static std::unique_ptr<ShaderManager> pShader;
    static std::unique_ptr<CommandPoolWrap> pCommandPool;
    static std::unique_ptr<DepthBuffer> pDepth;
public:
    VulkanContext() = delete;
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext(const VulkanContext&&) = delete;

    VulkanContext& operator=(const VulkanContext&) = delete;

    static void clear()
    {
        pInstance.reset(nullptr);
        pDevice.reset(nullptr);
        pQueue.reset(nullptr);
        pPhysicalDevice.reset(nullptr);
        pSwapchain.reset(nullptr);
        pSurface.reset(nullptr);
        pWindow.reset(nullptr);
        pSwapchainManager.reset(nullptr);
        pShader.reset(nullptr);
        pCommandPool.reset(nullptr);
        pDepth.reset(nullptr);
    }

    static InstanceWrap* instance()
    {
        return pInstance.get();
    }

    static DeviceWrap* device()
    {
        return pDevice.get();
    }

    static QueueWrap* queue()
    {
        return pQueue.get();
    }

    static PhysicalDeviceWrap* physicalDevice()
    {
        return pPhysicalDevice.get();
    }

    static SwapchainWrap* swapchain()
    {
        return pSwapchain.get();
    }

    static SwapchainResourceManager* swapchainManager()
    {
        return pSwapchainManager.get();
    }

    static SurfaceWrap* surface()
    {
        return pSurface.get();
    }

    static WindowWrap* window()
    {
        return pWindow.get();
    }

    static ShaderManager* shader()
    {
        return pShader.get();
    }

    static CommandPoolWrap* commandPool()
    {
        return pCommandPool.get();
    }

    static DepthBuffer* depth()
    {
        return pDepth.get();
    }

    static void reg(InstanceWrap* wrap)
    {
        pInstance.reset(wrap);
    }

    static void reg(DeviceWrap* wrap)
    {
        pDevice.reset(wrap);
    }

    static void reg(QueueWrap* wrap)
    {
        pQueue.reset(wrap);
    }

    static void reg(PhysicalDeviceWrap* wrap)
    {
        pPhysicalDevice.reset(wrap);
    }

    static void reg(SwapchainWrap* wrap)
    {
        pSwapchain.reset(wrap);
    }

    static void reg(SwapchainResourceManager* wrap)
    {
        pSwapchainManager.reset(wrap);
    }

    static void reg(SurfaceWrap* wrap)
    {
        pSurface.reset(wrap);
    }

    static void reg(WindowWrap* wrap)
    {
        pWindow.reset(wrap);
    }

    static void reg(ShaderManager* wrap)
    {
        pShader.reset(wrap);
    }

    static void reg(CommandPoolWrap* wrap)
    {
        pCommandPool.reset(wrap);
    }

    static void reg(DepthBuffer* wrap)
    {
        pDepth.reset(wrap);
    }
};
