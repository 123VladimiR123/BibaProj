#ifndef BIBAPROJ_QUEUEWRAP_H
#define BIBAPROJ_QUEUEWRAP_H


#include <memory>
#include <vulkan/vulkan_core.h>
#include "DeviceWrap.h"

class QueueWrap
{
    std::shared_ptr<VkQueue> present;
    std::shared_ptr<VkQueue> graphics;
    std::shared_ptr<VkQueue> compute;
    std::shared_ptr<VkQueue> transfer;

public:
    QueueWrap(const QueueWrap&) = delete;

    QueueWrap& operator=(const QueueWrap&) = delete;

    QueueWrap(VkDevice* device, QueueFamilyIndices* info);

    [[nodiscard]] VkQueue* getPresent() const { return present.get(); }
    [[nodiscard]] VkQueue* getGraphics() const { return graphics.get(); }
    [[nodiscard]] VkQueue* getTransfer() const { return transfer.get(); }
    [[nodiscard]] VkQueue* getCompute() const { return compute.get(); }
};


#endif //BIBAPROJ_QUEUEWRAP_H
