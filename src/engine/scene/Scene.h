//
// Created by admin on 8/20/2025.
//

#ifndef SCENE_H
#define SCENE_H
#include <memory>
#include <vector>

#include "./../gameobj/GameObject.h"
#include "./../gameobj/camera/Camera.h"
#include "../visual/MeshBufferManager.h"

class Scene
{
public:
    std::vector<std::shared_ptr<GameObject>> gameObjsRoot;
    std::vector<std::shared_ptr<GameObject>> gameObjsFlat;
    std::vector<std::shared_ptr<Camera>> cameras;
    std::shared_ptr<Camera> activeCamera;

    std::vector<std::shared_ptr<Material>> allMaterials;
    std::vector<std::shared_ptr<Texture>> allTextures;
    std::vector<std::shared_ptr<MeshPrimitive>> allMeshes;

    std::unordered_map<uint_fast8_t, std::vector<std::shared_ptr<Material>>> groupedMaterials;

    std::unique_ptr<VkDescriptorSetLayout> descLayout;
    std::unique_ptr<MeshBufferManager> bufferManager;
    std::unique_ptr<VkRenderPass, std::function<void(VkRenderPass*)>> renderPass;
    std::unique_ptr<VkDescriptorPool, std::function<void(VkDescriptorPool*)>> descriptorPool;

    Scene() = default;
    Scene(Scene&) = delete;

    Scene(Scene&&) noexcept = default;

    Scene& operator=(Scene&) = delete;
    Scene& operator=(Scene&&) noexcept = default;

    std::vector<std::shared_ptr<VkDescriptorSetLayout>> getDesctiptorLayouts() const;
};

#endif //SCENE_H
