//
// Created by admin on 8/27/2025.
//

#ifndef RESOURCEFILLER_H
#define RESOURCEFILLER_H
#include <vector>

#include "MeshBufferManager.h"
#include "../gameobj/nested/Material.h"
#include "../gameobj/nested/Texture.h"

class MeshPrimitive;
class Scene;

class ResourceFiller
{
    static std::shared_ptr<Texture> defaultTexture;
public:
    static void fillTexture(VkDevice* device, VkPhysicalDevice* physicalDevice, VkCommandPool* commandPool,
                            VkQueue* graphicsQueue, Texture& texture);
    static void fillAllTextures(VkDevice* device, VkPhysicalDevice* physicalDevice, VkCommandPool* commandPool,
                                VkQueue* graphicsQueue, std::vector<std::shared_ptr<Texture>>& textures);
    static void fillAllMaterialsBeforeDescPool(VkDevice* device, Scene* scene, VkPhysicalDevice* physicalDevice, VkCommandPool* commandPool, VkQueue* graphicsQueue);
    static void fillDescriptorPool(Scene*, VkDevice* device);
    static std::shared_ptr<Texture> makeDefaultTexture(VkDevice* device, VkPhysicalDevice* physicalDevice,
                                                       VkCommandPool* commandPool, VkQueue* graphicsQueue);
    static void fillAllMaterialsAfterDescPool(std::vector<std::shared_ptr<Material>>& materials,
                                              VkDescriptorPool* pool, VkDevice* device);
    static std::unique_ptr<MeshBufferManager> createSceneBufferManager(
        std::vector<std::shared_ptr<MeshPrimitive>>& allMeshes, VkDevice* device, VkPhysicalDevice* physicalDevice,
        VkCommandPool* commandPool, VkQueue* transferQueue);
    static void makePipelines(Scene* scene, VkDevice* device, VkPhysicalDevice* physicalDevice,
                              VkSurfaceFormatKHR* format, std::vector<VkPipelineShaderStageCreateInfo> shaderStages,
                              VkExtent2D* extent, VkDescriptorSetLayout* uboLayout);
    static void resetVkDependencies(Scene* scene, VkDevice* device);
};

#endif //RESOURCEFILLER_H
