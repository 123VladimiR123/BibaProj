//
// Created by admin on 8/21/2025.
//

#ifndef SCENELOADER_H
#define SCENELOADER_H
#include "tiny_gltf.h"

#include "SceneContainer.h"


class Camera;

class SceneLoader
{
    static void mapScenes(const tinygltf::Model*, SceneContainter*);

    static std::vector<float> getFloatData(const tinygltf::Model&, int);
    static void parseVerticesAndIndices(const tinygltf::Model& model, const tinygltf::Primitive& prim,
                                        std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    static std::vector<std::shared_ptr<Texture>> parseTextures(const tinygltf::Model*);

    static std::vector<std::shared_ptr<Material>> parseMaterials(const tinygltf::Model*,
                                                                 const std::vector<std::shared_ptr<Texture>>&);

    static std::shared_ptr<Camera> parseCamera(const tinygltf::Model* model, const tinygltf::Node& node);

    static void collectSceneResources(std::vector<std::shared_ptr<Material>>&,
                                      std::vector<std::shared_ptr<MeshPrimitive>>&,
                                      std::vector<std::shared_ptr<Texture>>&,
                                      std::vector<std::shared_ptr<GameObject>>&);

    static void parseGameObjects(
        const tinygltf::Model*, SceneContainter*);

    static Transform parseTransform(const tinygltf::Node& node);

public:
    SceneLoader() = delete;
    SceneLoader(SceneLoader&) = delete;
    SceneLoader(SceneLoader&&) = delete;
    void operator=(SceneLoader other) = delete;

    static void loadByDir(const std::string&, SceneContainter*);
};

#endif //SCENELOADER_H
