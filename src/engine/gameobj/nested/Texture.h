//
// Created by admin on 8/19/2025.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include <functional>
#include <memory>
#include <vulkan/vulkan_core.h>

struct SamplerInfo {
    VkFilter magFilter   = VK_FILTER_LINEAR;
    VkFilter minFilter   = VK_FILTER_LINEAR;
    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
};

class Texture {
public:
    std::string name;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 4;

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    bool isSRGB = true;

    SamplerInfo samplerInfo;

    std::vector<unsigned char> pixels;

    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

    Texture() = default;
    Texture(Texture&) = delete;

    Texture(Texture&&) noexcept = default;

    Texture& operator=(Texture&) = delete;
    Texture& operator=(Texture&&) noexcept = default;
};
#endif //TEXTURE_H
