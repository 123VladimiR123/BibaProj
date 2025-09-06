//
// Created by Владимир on 08.08.2025.
//

#include "QueueWrap.h"
#include "../util/Logger.h"

QueueWrap::QueueWrap(VkDevice* device, QueueFamilyIndices* info)
{
    if (!info->isComplete())
        throw std::runtime_error("Presentation or graphics queue is not supported");

    VkQueue graph{};
    vkGetDeviceQueue(*device, info->graphicsFamily.value(), 0, &graph);
    graphics = std::make_shared<VkQueue>(graph);

    VkQueue pres{};
    vkGetDeviceQueue(*device, info->presentFamily.value(), 0, &pres);
    present = std::make_shared<VkQueue>(pres);

    if (info->computeFamily.has_value())
    {
        VkQueue comp{};
        vkGetDeviceQueue(*device, info->computeFamily.value(), 0, &comp);
        compute = std::make_shared<VkQueue>(comp);
    }
    else
    {
        compute = graphics;
    }

    if (info->transferFamily.has_value())
    {
        VkQueue trans{};
        vkGetDeviceQueue(*device, info->transferFamily.value(), 0, &trans);
        transfer = std::make_shared<VkQueue>(trans);
    }
    else
    {
        transfer = graphics;
    }

    Logger::get() << std::endl << "Queues initialized, families: " << std::endl;
    Logger::get() << "Graphics: " << info->graphicsFamily.value() << std::endl;
    Logger::get() << "Present: " << info->presentFamily.value() << std::endl;
    Logger::get() << "Compute: " << info->computeFamily.value_or(info->graphicsFamily.value()) << std::endl;
    Logger::get() << "Transfer: " << info->transferFamily.value_or(info->graphicsFamily.value()) << std::endl;
}
