//
// Created by admin on 8/28/2025.
//

#ifndef COMMANDPOOLWRAP_H
#define COMMANDPOOLWRAP_H
#include <memory>
#include <vulkan/vulkan_core.h>

#include "util/QueueFamilyIndices.h"


class CommandPoolWrap
{
    std::shared_ptr<VkCommandPool> graphicsPool;
    std::shared_ptr<VkCommandPool> presentPool;
    std::shared_ptr<VkCommandPool> transferPool;
    std::shared_ptr<VkCommandPool> computePool;

public:
    CommandPoolWrap(const VkDevice*, const QueueFamilyIndices*);

    [[nodiscard]] VkCommandPool* getGraphics() const { return graphicsPool.get(); }
    [[nodiscard]] VkCommandPool* getPresent() const { return presentPool.get(); }
    [[nodiscard]] VkCommandPool* getTransfer() const { return transferPool.get(); }
    [[nodiscard]] VkCommandPool* getCompute() const { return computePool.get(); }

    void destroyPools();
};


#endif //COMMANDPOOLWRAP_H
