//
// Created by admin on 8/22/2025.
//

#ifndef SCENECONTAINTER_H
#define SCENECONTAINTER_H
#include <memory>
#include <vector>

#include "Scene.h"

class SceneContainter
{
    bool finished = false;
    std::shared_ptr<Scene> activeScene;

public:
    std::vector<std::shared_ptr<Scene>> scenes;

    std::vector<std::shared_ptr<Material>> allMaterials;
    std::vector<std::shared_ptr<Texture>> allTextures;
    std::vector<std::shared_ptr<GameObject>> allObjects;
    std::vector<std::shared_ptr<Camera>> allCameras;
    std::vector<std::shared_ptr<MeshPrimitive>> allMeshes;

    void setActiveScene(const size_t index)
    {
        activeScene = scenes[index];
    }

    void setActiveScene(const Scene* ptr)
    {
        for (const std::shared_ptr<Scene>& loadedSceneLink : scenes)
            if (loadedSceneLink.get() == ptr)
                activeScene = loadedSceneLink;

        if (activeScene == nullptr)
            throw std::runtime_error("Scene wasn't found");
    }

    Scene* getActiveScene()
    {
        return activeScene.get();
    }

    SceneContainter() = default;
    SceneContainter(SceneContainter&) = delete;

    SceneContainter(SceneContainter&&) noexcept = default;

    SceneContainter& operator=(SceneContainter&) = delete;
    SceneContainter& operator=(SceneContainter&&) noexcept = default;

    friend class SceneLoader;
};

#endif //SCENECONTAINTER_H
