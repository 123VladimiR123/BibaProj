//
// Created by Владимир on 08.08.2025.
//

#include <stdexcept>
#include <iostream>
#include "PhysicalDeviceWrap.h"
#include "../util/Cfg.h"
#include "../util/Logger.h"

PhysicalDeviceWrap::PhysicalDeviceWrap(VkInstance *instance) {
    uint32_t deviceCounter{};
    vkEnumeratePhysicalDevices(*instance, &deviceCounter, nullptr);

    if (!deviceCounter)
        throw std::runtime_error("Devices that support Vulkan API not found");

    auto *allDevices = new VkPhysicalDevice[deviceCounter];
    vkEnumeratePhysicalDevices(*instance, &deviceCounter, allDevices);

    Logger::get() << std::endl << "Available devices:" << std::endl;

    VkPhysicalDeviceProperties deviceProperties;
    for (uint32_t i{0}; i < deviceCounter; i++) {
        vkGetPhysicalDeviceProperties(allDevices[i], &deviceProperties);
        Logger::get() << deviceProperties.deviceName << " <<< Type: " << deviceProperties.deviceType << " <<< ID: "
                      << deviceProperties.deviceID << std::endl;
        if (deviceProperties.deviceID == Cfg::selectedGPUID)
            device = allDevices[i];
    }

    if (device == nullptr)
    {
        for (uint32_t i{0}; i < deviceCounter; i++) {
            vkGetPhysicalDeviceProperties(allDevices[i], &deviceProperties);
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                device = allDevices[i];
        }

        if (device == nullptr) {
            if (deviceCounter > 0) {
                device = allDevices[0];
                delete[] allDevices;
            } else {
                delete[] allDevices;
                throw std::runtime_error("Suitable device wasn't found");
            }
        }
    }

    vkGetPhysicalDeviceFeatures(device, &features);
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    Logger::get() << std::endl
        << "Selected device: " << std::endl
        <<  deviceProperties.deviceName << std::endl
        << "Driver version: " << deviceProperties.driverVersion << std::endl;
}
