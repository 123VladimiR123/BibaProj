//
// Created by admin on 8/19/2025.
//

#ifndef MATERIAL_H
#define MATERIAL_H
#include <memory>
#include <string>
#include <vulkan/vulkan_core.h>

#include "Texture.h"
#include "./../../maths/Vector3.h"
#include "./../../maths/Vector4.h"

enum AlphaMode { OPAQUE_MODE, MASK_MODE, BLEND_MODE };

class Material
{
public:
    std::string name;
    std::shared_ptr<VkDescriptorSet> descriptor;
    std::shared_ptr<VkDescriptorSetLayout> descriptorLayout;
    std::shared_ptr<VkPipeline> pipeline;
    std::shared_ptr<VkPipelineLayout> pipelineLayout;

    Vector4 albedoColor{1.0f, 1.0f, 1.0f, 1.0f};
    float roughness = 1.0f;
    float metallic = 1.0f;

    Vector3 emissiveColor{0.0f, 0.0f, 0.0f};
    float alphaCutoff = 0.5f;
    bool doubleSided = false;

    std::shared_ptr<Texture> albedoTexture;                 // 0b00001 - bind #0
    std::shared_ptr<Texture> metallicRoughnessTexture;      // 0b00010 - bind #1
    std::shared_ptr<Texture> normalTexture;                 // 0b00100 - bind #2
    std::shared_ptr<Texture> occlusionTexture;              // 0b01000 - bind #3
    std::shared_ptr<Texture> emissiveTexture;               // 0b10000 - bind #4

    AlphaMode alphaMode = OPAQUE_MODE;

    Material() = default;
    Material(Material&) = delete;

    Material(Material&&) noexcept = default;

    Material& operator=(Material&) = delete;
    Material& operator=(Material&&) noexcept = default;
};

#endif //MATERIAL_H
