#include <memory>
#include <set>
#include <limits>
#include "DeviceWrap.h"
#include "../util/Logger.h"

DeviceWrap::DeviceWrap(VkSurfaceKHR* surface, VkPhysicalDevice* physicalDevice, VkPhysicalDeviceFeatures &features)
{
    defineQueueIndices(*physicalDevice, *surface);

    if (!indices->isComplete())
        throw std::runtime_error("Queue isn't supporting presentation or graphics");

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const std::set uniqueQueueFamilies = {
        indices->graphicsFamily.value(),
        indices->presentFamily.value(),
        indices->computeFamily.value_or(indices->graphicsFamily.value()),
        indices->transferFamily.value_or(indices->graphicsFamily.value())
    };

    float queuePriority = 1.0f;
    for (const uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = 1;
    createInfo.pEnabledFeatures = &features;

    const char* exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME};
    createInfo.ppEnabledExtensionNames = exts;

    if (vkCreateDevice(*physicalDevice, &createInfo, nullptr, &device) == VK_SUCCESS)
    {
        Logger::get() << std::endl << "Logical device created successfully" << std::endl;
    }
    else
        throw std::runtime_error("Device creation failed on VkApi vkCreateDevice(...)");
}

void DeviceWrap::defineQueueIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    indices = std::make_unique<QueueFamilyIndices>();

    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    if (!familyCount) throw std::runtime_error("An error occurred on queue defining");

    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());

    Logger::get() << std::endl << "Queues total: " << familyCount << std::endl;
    for (uint32_t i{}; i < familyCount; i++)
    {
        Logger::get() << families[i].queueCount << " - queue count; "
            << "0x"
            << std::hex
            << std::setw(8)
            << std::setfill('0')
            << families[i].queueFlags
            << " - bit flags"
            << std::endl;
    }

    int bestComputeScore = std::numeric_limits<int>::min();
    int bestTransferScore = std::numeric_limits<int>::min();

    for (uint32_t i = 0; i < familyCount; i++)
    {
        const auto& props = families[i];
        VkQueueFlags flags = props.queueFlags;

        if ((flags & VK_QUEUE_GRAPHICS_BIT) && !indices->graphicsFamily.has_value())
        {
            indices->graphicsFamily = i;
        }

        if (flags & VK_QUEUE_COMPUTE_BIT)
        {
            int score = 0;
            if (!(flags & VK_QUEUE_GRAPHICS_BIT)) score += 100;
            if (!(flags & VK_QUEUE_TRANSFER_BIT)) score += 10;

            score += props.queueCount;

            if (score > bestComputeScore)
            {
                bestComputeScore = score;
                indices->computeFamily = i;
            }
        }

        if (flags & VK_QUEUE_TRANSFER_BIT)
        {
            int score = 0;
            if (!(flags & VK_QUEUE_GRAPHICS_BIT)) score += 100;
            if (!(flags & VK_QUEUE_COMPUTE_BIT)) score += 10;

            score += props.queueCount;

            if (score > bestTransferScore)
            {
                bestTransferScore = score;
                indices->transferFamily = i;
            }
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport && !indices->presentFamily.has_value())
        {
            indices->presentFamily = i;
        }
    }


    if (!indices->isComplete())
        throw std::runtime_error("Queue indecies weren't found");
}
