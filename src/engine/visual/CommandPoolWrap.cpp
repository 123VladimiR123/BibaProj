//
// Created by admin on 8/28/2025.
//

#include "CommandPoolWrap.h"

#include "../util/Logger.h"

CommandPoolWrap::CommandPoolWrap(const VkDevice* device, const QueueFamilyIndices* indices)
{
    if (!indices->isComplete())
        throw std::runtime_error("Indices are not completed");

    auto commandPoolDeleter = [device](const VkCommandPool* pool)
    {
        if (pool)
        {
            vkDestroyCommandPool(*device, *pool, nullptr);
            delete pool;
        }
    };

    // graphics
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices->graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    graphicsPool = std::shared_ptr<VkCommandPool>(new VkCommandPool(), commandPoolDeleter);
    if (vkCreateCommandPool(*device, &poolInfo, nullptr, graphicsPool.get()) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool for graphics queue");

    // present
    if (indices->presentFamily == indices->graphicsFamily)
    {
        presentPool = graphicsPool;
    }
    else
    {
        poolInfo.queueFamilyIndex = indices->presentFamily.value();
        presentPool = std::shared_ptr<VkCommandPool>(new VkCommandPool(), commandPoolDeleter);
        if (vkCreateCommandPool(*device, &poolInfo, nullptr, presentPool.get()) != VK_SUCCESS)
            throw std::runtime_error("Failed to create command pool for present queue");
    }

    //transfer
    if (!indices->transferFamily.has_value())
    {
        transferPool = graphicsPool;
    }
    else
    {
        if (indices->transferFamily == indices->graphicsFamily)
        {
            transferPool = graphicsPool;
        }
        else if (indices->transferFamily == indices->presentFamily)
        {
            transferPool = presentPool;
        }
        else
        {
            poolInfo.queueFamilyIndex = indices->transferFamily.value();
            transferPool = std::shared_ptr<VkCommandPool>(new VkCommandPool(), commandPoolDeleter);
            if (vkCreateCommandPool(*device, &poolInfo, nullptr, transferPool.get()) != VK_SUCCESS)
                throw std::runtime_error("Failed to create command pool for transfer queue");
        }
    }

    //compute
    if (!indices->computeFamily.has_value())
    {
        computePool = graphicsPool;
    }
    else
    {
        if (indices->computeFamily == indices->graphicsFamily)
        {
            computePool = graphicsPool;
        }
        else if (indices->computeFamily == indices->presentFamily)
        {
            computePool = presentPool;
        }
        else if (indices->computeFamily == indices->transferFamily)
        {
            computePool = transferPool;
        }
        else
        {
            poolInfo.queueFamilyIndex = indices->computeFamily.value();
            computePool = std::shared_ptr<VkCommandPool>(new VkCommandPool(), commandPoolDeleter);
            if (vkCreateCommandPool(*device, &poolInfo, nullptr, computePool.get()) != VK_SUCCESS)
                throw std::runtime_error("Failed to create command pool for compute queue");
        }
    }

    Logger::get() << std::endl << "Command pools created" << std::endl;
}

void CommandPoolWrap::destroyPools()
{
    graphicsPool.reset();
    presentPool.reset();
    transferPool.reset();
    computePool.reset();
}