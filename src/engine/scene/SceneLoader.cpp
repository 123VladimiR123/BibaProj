//
// Created by admin on 8/21/2025.
//
#include <nlohmann/json.hpp>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_INCLUDE_RAPIDJSON
#define TINYGLTF_USE_NLOHMANN_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "SceneLoader.h"

#include <iostream>

#include "../gameobj/camera/Camera.h"
#include "../gameobj/camera/PerspectiveCamera.h"
#include "../gameobj/camera/OrthographicCamera.h"
#include "../util/Logger.h"
#include "../util/ProjectPath.h"
#include "../util/Cfg.h"

std::vector<std::shared_ptr<Texture>> SceneLoader::parseTextures(const tinygltf::Model* model)
{
    std::vector<std::shared_ptr<Texture>> textures;
    textures.reserve(model->textures.size());

    for (const auto& tex : model->textures)
    {
        if (tex.source < 0 || tex.source >= static_cast<int>(model->images.size()))
            continue;

        const auto& img = model->images[tex.source];
        auto texture = std::make_shared<Texture>();

        texture->name = tex.name.empty() ? img.name : tex.name;
        texture->width = img.width;
        texture->height = img.height;
        texture->channels = img.component;

        if (img.bits == 8)
        {
            if (texture->channels == 1) texture->format = VK_FORMAT_R8_UNORM;
            if (texture->channels == 2) texture->format = VK_FORMAT_R8G8_UNORM;
        }
        else if (img.bits == 16)
        {
            if (texture->channels == 1) texture->format = VK_FORMAT_R16_UNORM;
            else if (texture->channels == 2) texture->format = VK_FORMAT_R16G16_UNORM;
            else texture->format = VK_FORMAT_R16G16B16A16_UNORM;
        }

        texture->isSRGB = true;
        texture->pixels = img.image;

        if (tex.sampler >= 0 && tex.sampler < static_cast<int>(model->samplers.size()))
        {
            const tinygltf::Sampler* smp = nullptr;
            if (tex.sampler >= 0 && tex.sampler < static_cast<int>(model->samplers.size()))
            {
                smp = &model->samplers[tex.sampler];
            }

            if (smp && smp->minFilter >= 0)
            {
                switch (smp->minFilter)
                {
                case TINYGLTF_TEXTURE_FILTER_NEAREST:
                    texture->samplerInfo.minFilter = VK_FILTER_NEAREST;
                    break;
                default:
                    texture->samplerInfo.minFilter = VK_FILTER_LINEAR;
                    break;
                }
            }

            if (smp && smp->magFilter >= 0)
            {
                switch (smp->magFilter)
                {
                case TINYGLTF_TEXTURE_FILTER_NEAREST:
                    texture->samplerInfo.magFilter = VK_FILTER_NEAREST;
                    break;
                default:
                    texture->samplerInfo.magFilter = VK_FILTER_LINEAR;
                    break;
                }
            }

            if (smp)
            {
                switch (smp->wrapS)
                {
                case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: texture->samplerInfo.addressModeU =
                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                    break;
                case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: texture->samplerInfo.addressModeU =
                        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                    break;
                default: texture->samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                    break;
                }

                switch (smp->wrapT)
                {
                case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: texture->samplerInfo.addressModeV =
                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                    break;
                case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: texture->samplerInfo.addressModeV =
                        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                    break;
                default: texture->samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                    break;
                }
            }
        }

        textures.push_back(std::move(texture));
    }

    return textures;
}


float safeNormalizedSigned(int32_t val, int32_t maxVal) {
    if (val == -maxVal) return -1.0f;
    return static_cast<float>(val) / static_cast<float>(maxVal);
}

std::vector<float> SceneLoader::getFloatData(const tinygltf::Model& model, const int accessorIdx) {
    const auto& accessor = model.accessors[accessorIdx];
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[bufferView.buffer];

    const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

    size_t elementSize = 0;
    switch (accessor.type) {
        case TINYGLTF_TYPE_SCALAR: elementSize = 1; break;
        case TINYGLTF_TYPE_VEC2: elementSize = 2; break;
        case TINYGLTF_TYPE_VEC3: elementSize = 3; break;
        case TINYGLTF_TYPE_VEC4: elementSize = 4; break;
        default:
            throw std::runtime_error("Unsupported accessor type");
    }

    std::vector<float> result(accessor.count * elementSize);

    size_t componentSize = 0;
    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: componentSize = 1; break;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: componentSize = 2; break;
        case TINYGLTF_COMPONENT_TYPE_INT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: componentSize = 4; break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT: componentSize = 4; break;
        default: throw std::runtime_error("Unknown componentType");
    }

    size_t stride = bufferView.byteStride ? bufferView.byteStride : componentSize * elementSize;
    const unsigned char* ptr = dataPtr;

    for (size_t i = 0; i < accessor.count; ++i) {
        for (size_t j = 0; j < elementSize; ++j) {
            switch (accessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_BYTE: {
                    int8_t val = *reinterpret_cast<const int8_t*>(ptr + j * componentSize);
                    result[i * elementSize + j] = accessor.normalized ? safeNormalizedSigned(val, 127) : float(val);
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                    uint8_t val = *(ptr + j * componentSize);
                    result[i * elementSize + j] = accessor.normalized ? float(val) / 255.0f : float(val);
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_SHORT: {
                    int16_t val = *reinterpret_cast<const int16_t*>(ptr + j * componentSize);
                    result[i * elementSize + j] = accessor.normalized ? safeNormalizedSigned(val, 32767) : float(val);
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                    uint16_t val = *reinterpret_cast<const uint16_t*>(ptr + j * componentSize);
                    result[i * elementSize + j] = accessor.normalized ? float(val) / 65535.0f : float(val);
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_INT: {
                    int32_t val = *reinterpret_cast<const int32_t*>(ptr + j * componentSize);
                    result[i * elementSize + j] = accessor.normalized ? safeNormalizedSigned(val, 2147483647) : float(val);
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                    uint32_t val = *reinterpret_cast<const uint32_t*>(ptr + j * componentSize);
                    result[i * elementSize + j] = accessor.normalized ? float(val) / 4294967295.0f : float(val);
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_FLOAT: {
                    float val = *reinterpret_cast<const float*>(ptr + j * componentSize);
                    result[i * elementSize + j] = val;
                    break;
                }
                default: throw std::runtime_error("Unknown componentType");
            }
        }
        ptr += stride;
    }

    return result;
}

std::vector<Vertex> SceneLoader::parseMeshPrimitive(const tinygltf::Model& model, const tinygltf::Primitive& prim)
{
    size_t vertexCount = 0;
    if (prim.attributes.contains("POSITION"))
        vertexCount = model.accessors[prim.attributes.at("POSITION")].count;

    std::vector<Vertex> vertices(vertexCount);

    if (prim.attributes.contains("POSITION"))
    {
        const std::vector<float> data = getFloatData(model, prim.attributes.at("POSITION"));
        for (size_t i = 0; i < vertexCount; ++i)
        {
            vertices[i].position = {data[i * 3 + 0], data[i * 3 + 1], data[i * 3 + 2]};
        }
    }

    if (prim.attributes.contains("NORMAL"))
    {
        const std::vector<float> data = getFloatData(model, prim.attributes.at("NORMAL"));
        for (size_t i = 0; i < vertexCount; ++i)
        {
            vertices[i].normal = {data[i * 3 + 0], data[i * 3 + 1], data[i * 3 + 2]};
        }
    }

    if (prim.attributes.contains("TANGENT"))
    {
        const std::vector<float> data = getFloatData(model, prim.attributes.at("TANGENT"));
        for (size_t i = 0; i < vertexCount; ++i)
        {
            vertices[i].tangent = {data[i * 4 + 0], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]};
        }
    }

    if (prim.attributes.contains("TEXCOORD_0"))
    {
        const std::vector<float> data = getFloatData(model, prim.attributes.at("TEXCOORD_0"));
        for (size_t i = 0; i < vertexCount; ++i)
        {
            vertices[i].uv = {data[i * 2 + 0], data[i * 2 + 1]};
        }
    }

    for (size_t i = 0; i < vertexCount; ++i)
    {
        auto& v = vertices[i];

        v.bitangent = {
            v.normal.y * v.tangent.z - v.normal.z * v.tangent.y,
            v.normal.z * v.tangent.x - v.normal.x * v.tangent.z,
            v.normal.x * v.tangent.y - v.normal.y * v.tangent.x
        };

        v.bitangent.x *= v.tangent.w;
        v.bitangent.y *= v.tangent.w;
        v.bitangent.z *= v.tangent.w;
    }

    return vertices;
}

std::vector<uint32_t> SceneLoader::parseMeshIndices(const tinygltf::Model& model, const tinygltf::Primitive& prim)
{
    std::vector<uint32_t> indices;

    if (prim.indices < 0)
    {
        if (prim.attributes.contains("POSITION"))
        {
            const auto& accessor = model.accessors[prim.attributes.at("POSITION")];
            indices.resize(accessor.count);
            for (size_t i = 0; i < accessor.count; ++i)
                indices[i] = static_cast<uint32_t>(i);
        }
        return indices;
    }

    const auto& accessor = model.accessors[prim.indices];
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[bufferView.buffer];

    const unsigned char* basePtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
    const size_t indexCount = accessor.count;
    indices.resize(indexCount);

    const size_t stride = bufferView.byteStride ? bufferView.byteStride :
                          (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE ? 1 :
                           accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ? 2 : 4);

    for (size_t i = 0; i < indexCount; ++i)
    {
        const unsigned char* ptr = basePtr + i * stride;
        switch (accessor.componentType)
        {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            {
                uint8_t val;
                memcpy(&val, ptr, sizeof(val));
                indices[i] = static_cast<uint32_t>(val);
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            {
                uint16_t val;
                memcpy(&val, ptr, sizeof(val));
                indices[i] = static_cast<uint32_t>(val);
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            {
                uint32_t val;
                memcpy(&val, ptr, sizeof(val));
                indices[i] = val;
                break;
            }
            default:
                throw std::runtime_error("Unsupported index component type in glTF");
        }
    }

    return indices;
}


std::vector<std::shared_ptr<Material>> SceneLoader::parseMaterials(
    const tinygltf::Model* model,
    const std::vector<std::shared_ptr<Texture>>& textures)
{
    std::vector<std::shared_ptr<Material>> result;

    for (const auto& mat : model->materials)
    {
        auto material = std::make_shared<Material>();
        material->name = mat.name;

        if (!mat.pbrMetallicRoughness.baseColorFactor.empty())
        {
            const auto& c = mat.pbrMetallicRoughness.baseColorFactor;
            material->albedoColor = Vector4{
                static_cast<float>(c[0]), static_cast<float>(c[1]), static_cast<float>(c[2]),
                static_cast<float>(c.size() > 3 ? c[3] : 1.0f)
            };
        }

        material->metallic = static_cast<float>(mat.pbrMetallicRoughness.metallicFactor);
        material->roughness = static_cast<float>(mat.pbrMetallicRoughness.roughnessFactor);

        if (!mat.emissiveFactor.empty())
        {
            const auto& e = mat.emissiveFactor;
            material->emissiveColor = Vector3{
                static_cast<float>(e[0]), static_cast<float>(e[1]), static_cast<float>(e[2])
            };
        }

        if (mat.alphaMode == "MASK") material->alphaMode = MASK_MODE;
        else if (mat.alphaMode == "BLEND") material->alphaMode = BLEND_MODE;

        material->alphaCutoff = static_cast<float>(mat.alphaCutoff);
        material->doubleSided = mat.doubleSided;

        // todo what should i do if texture wasn't found?
        const auto checkNSet = [textures](const int index) -> std::shared_ptr<Texture>
        {
            if (index == -1)
                return nullptr;
            if (index >= textures.size())
                throw std::runtime_error("Texture index is out of bounds");
            return textures[index];
        };

        material->albedoTexture = checkNSet(mat.pbrMetallicRoughness.baseColorTexture.index);
        material->metallicRoughnessTexture = checkNSet(mat.pbrMetallicRoughness.metallicRoughnessTexture.index);
        material->normalTexture = checkNSet(mat.normalTexture.index);
        material->occlusionTexture = checkNSet(mat.occlusionTexture.index);
        material->emissiveTexture = checkNSet(mat.emissiveTexture.index);

        if (material->normalTexture != nullptr)
            material->normalTexture->isSRGB = false;
        if (material->metallicRoughnessTexture != nullptr)
            material->metallicRoughnessTexture->isSRGB = false;
        if (material->occlusionTexture != nullptr)
            material->occlusionTexture->isSRGB = false;

        result.push_back(material);
    }

    return result;
}

std::shared_ptr<Camera> SceneLoader::parseCamera(const tinygltf::Model* model, const tinygltf::Node& node)
{
    if (model->cameras[node.camera].type == "perspective")
    {
        const auto camPersp = std::make_shared<PerspectiveCamera>();
        const tinygltf::PerspectiveCamera& cam = model->cameras[node.camera].perspective;

        camPersp->transform = parseTransform(node);
        camPersp->aspect = static_cast<float>(cam.aspectRatio);
        camPersp->fov = static_cast<float>(cam.yfov);
        camPersp->nearPlane = static_cast<float>(cam.znear);
        camPersp->farPlane = static_cast<float>(cam.zfar);

        return std::static_pointer_cast<Camera>(camPersp);
    }
    if (model->cameras[node.camera].type == "orthographic")
    {
        const auto camOrth = std::make_shared<OrthographicCamera>();
        const tinygltf::OrthographicCamera& cam = model->cameras[node.camera].orthographic;

        camOrth->transform = parseTransform(node);
        camOrth->xmag = static_cast<float>(cam.xmag);
        camOrth->ymag = static_cast<float>(cam.ymag);
        camOrth->nearPlane = static_cast<float>(cam.znear);
        camOrth->farPlane = static_cast<float>(cam.zfar);

        return std::static_pointer_cast<Camera>(camOrth);
    }
    throw std::runtime_error("Camera type wasn't recognized. Actual value: " + model->cameras[node.camera].type);
}

void SceneLoader::parseGameObjects(
    const tinygltf::Model* model, SceneContainter* container)
{
    std::vector<std::vector<std::shared_ptr<MeshPrimitive>>> meshes;
    const std::vector<std::shared_ptr<Texture>> textures = parseTextures(model);
    std::vector<std::shared_ptr<Material>> materials = parseMaterials(model, textures);

    auto defaultMaterial = std::make_shared<Material>();
    materials.push_back(defaultMaterial);

    std::vector<std::shared_ptr<GameObject>> allGameObjs;
    std::vector<std::shared_ptr<Camera>> allCameras;

    Logger::get() << "Total meshes in raw scene: " << model->meshes.size();
    for (const auto& mesh : model->meshes)
    {
        std::vector<std::shared_ptr<MeshPrimitive>> current;
        for (const auto& prim : mesh.primitives)
        {
            auto currentMesh = std::make_shared<MeshPrimitive>();
            currentMesh->vertices = parseMeshPrimitive(*model, prim);
            currentMesh->indices = parseMeshIndices(*model, prim);
            if (0 <= prim.material && prim.material < materials.size())
                currentMesh->material = materials[prim.material];
            else
                currentMesh->material = defaultMaterial;

            current.push_back(currentMesh);
        }
        meshes.push_back(current);
    }

    for (size_t i = 0; i < model->nodes.size(); ++i)
    {
        if (const auto& node = model->nodes[i]; node.camera > -1)
        {
            const auto cam = parseCamera(model, node);
            cam->nodeIdx = i;
            allCameras.push_back(cam);
        }
        else
        {
            const auto obj = std::make_shared<GameObject>();
            Transform t = parseTransform(node);
            obj->transform = std::make_unique<Transform>(std::move(t));
            if (node.mesh >= 0 && node.mesh < meshes.size())
                obj->primitives = meshes[node.mesh];
            obj->nodeIdx = i;
            allGameObjs.push_back(obj);
        }
    }

    std::vector<std::shared_ptr<MeshPrimitive>> allMeshes;
    for (const auto& vec : allGameObjs)
    {
        allMeshes.insert(allMeshes.end(), vec->primitives.begin(), vec->primitives.end());
    }

    container->allMeshes = allMeshes;
    container->allCameras = allCameras;
    container->allMaterials = materials;
    container->allTextures = textures;
    container->allObjects = allGameObjs;

    uint32_t empties{};
    for (const auto& obj : allGameObjs)
        if (obj == nullptr)
            empties++;

    if (empties > 0)
        Logger::get() << empties << " is amount of empty objects" << std::endl;
}

void SceneLoader::collectSceneResources(std::vector<std::shared_ptr<Material>>& matVector,
                                        std::vector<std::shared_ptr<MeshPrimitive>>& meshVector,
                                        std::vector<std::shared_ptr<Texture>>& textVector,
                                        std::vector<std::shared_ptr<GameObject>>& currentObjs)
{
    for (const auto& obj : currentObjs)
    {
        if (obj == nullptr)
            continue;
        for (const auto& mesh : obj->primitives)
        {
            meshVector.push_back(mesh);
            if (mesh->material == nullptr)
                continue;
            matVector.push_back(mesh->material);
            if (mesh->material->albedoTexture != nullptr)
                textVector.push_back(mesh->material->albedoTexture);
            if (mesh->material->emissiveTexture != nullptr)
                textVector.push_back(mesh->material->emissiveTexture);
            if (mesh->material->normalTexture != nullptr)
                textVector.push_back(mesh->material->normalTexture);
            if (mesh->material->occlusionTexture != nullptr)
                textVector.push_back(mesh->material->occlusionTexture);
        }
        if (!obj->children.empty())
            collectSceneResources(matVector, meshVector, textVector, obj->children);
    }
}

void flatObjects(std::vector<std::shared_ptr<GameObject>> *src, std::vector<std::shared_ptr<GameObject>> *dest)
{
    for (const std::shared_ptr<GameObject>& oneSrc: *src)
    {
        if (oneSrc == nullptr) continue; //why?
        dest->push_back(oneSrc);
        if (!oneSrc->children.empty())
            flatObjects(&oneSrc->children, dest);
    }
}

void SceneLoader::mapScenes(const tinygltf::Model* model, SceneContainter* container)
{
    parseGameObjects(model, container);

    std::unordered_map<uint32_t, std::shared_ptr<GameObject>> objectsByidx;
    std::unordered_map<uint32_t, std::shared_ptr<Camera>> camerasByIdx;
    for (const auto& obj : container->allObjects)
        objectsByidx[obj->nodeIdx] = obj;
    for (const auto& cam : container->allCameras)
        camerasByIdx[cam->nodeIdx] = cam;

    for (const auto& obj : container->allObjects)
    {
        if (model->nodes[obj->nodeIdx].children.empty())
            continue;
        obj->children.resize(model->nodes[obj->nodeIdx].children.size());
        for (const auto idx : model->nodes[obj->nodeIdx].children)
        {
            if (!objectsByidx.contains(idx))
                continue;
            if (objectsByidx[idx]->parent != nullptr)
                throw std::runtime_error("Node multiparent found. Node index: " + idx);
            obj->children.push_back(objectsByidx[idx]);
            objectsByidx[idx]->parent = obj;
        }
    }

    for (const auto& scene : model->scenes)
    {
        auto scenePtr = std::make_shared<Scene>();
        for (const auto nodeIdx : scene.nodes)
        {
            if (objectsByidx.contains(nodeIdx))
                scenePtr->gameObjsRoot.push_back(objectsByidx[nodeIdx]);
            if (camerasByIdx.contains(nodeIdx))
                scenePtr->cameras.push_back(camerasByIdx[nodeIdx]);
        }

        if (scenePtr->cameras.empty())
            Logger::get() << std::endl << "Be careful. Scene contains no cameras" << std::endl;
        else if (scenePtr->cameras.size() > 1)
            Logger::get() << std::endl << "Scene contains few cameras. Please, select camera by yourself" << std::endl;
        else
            scenePtr->activeCamera = scenePtr->cameras[0];

        container->scenes.push_back(scenePtr);
    }

    if (container->scenes.empty())
        Logger::get() << std::endl << "Be careful. No scenes were mapped" << std::endl;
    else if (container->scenes.size() > 1)
        Logger::get() << std::endl << "More than one scene were found. Please, select scene by yourself" << std::endl;
    else
        container->setActiveScene(container->scenes[0].get());

    for (const auto& scene : container->scenes)
    {
        collectSceneResources(scene->allMaterials, scene->allMeshes, scene->allTextures, scene->gameObjsRoot);
    }

    for (const auto& scene : container->scenes)
    {
        flatObjects(&scene->gameObjsRoot, &scene->gameObjsFlat);
    }
}

bool imageLoader(
    tinygltf::Image* image,
    int image_idx,
    std::string* err,
    std::string* warn,
    int req_width,
    int req_height,
    const unsigned char* bytes,
    int size,
    void* user_data)
{
    int width, height, channels;

    unsigned char* data = stbi_load_from_memory(bytes, size, &width, &height, &channels, STBI_rgb_alpha);
    if (!data)
    {
        if (err) *err = "Error occured during image opening";
        return false;
    }

    image->width = width;
    image->height = height;
    image->component = 4;
    image->bits = 8;
    image->image.assign(data, data + width * height * 4);

    stbi_image_free(data);

    return true;
}

void SceneLoader::loadByDir(const std::string& dir, SceneContainter* containter)
{
    if (containter == nullptr)
        throw std::runtime_error("Scene container must exists");
    if (containter->finished)
        throw std::runtime_error("Scene was mapped before. Create a new one");

    const std::string sceneFullPath = ProjectPath::root.string() + "/" + Cfg::sceneParentDir + "/" + dir;
    std::vector<std::string> sceneFilenames;

    bool isBinary = false;
    for (const auto& file : std::filesystem::directory_iterator(sceneFullPath))
    {
        if (file.path().has_extension() && file.path().extension() == ".glb")
        {
            sceneFilenames.push_back(file.path().filename().string());
            isBinary = true;
        }
        else if (file.path().has_extension() && file.path().extension() == ".gltf")
            sceneFilenames.push_back(file.path().filename().string());
    }

    if (sceneFilenames.size() != 1)
        throw std::runtime_error("Only 1 scene must be in scene folder");

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    loader.SetImageLoader(imageLoader, nullptr);
    const bool success = isBinary
                             ? loader.LoadBinaryFromFile(&model, &err, &warn, sceneFullPath + "/" + sceneFilenames[0])
                             : loader.LoadASCIIFromFile(&model, &err, &warn, sceneFullPath + "/" + sceneFilenames[0]);

    if (!warn.empty())
        Logger::get() << "Warning occured during scene loading: " << warn << std::endl;
    if (!err.empty())
        throw std::runtime_error("Error occured during scene loading: " + err);
    if (!success)
        throw std::runtime_error("Unable to open scene");

    mapScenes(&model, containter);

    Logger::get() << std::endl << "Scene '" << dir << "' opened" << std::endl;
    Logger::get() << "Total textures: " << containter->allTextures.size() << std::endl;
    Logger::get() << "Total materials: " << containter->allMaterials.size() << std::endl;
    Logger::get() << "Total scenes: " << containter->scenes.size() << std::endl;
    Logger::get() << "Total objects: " << containter->allObjects.capacity() << std::endl;
    Logger::get() << "Total cameras: " << containter->allCameras.size() << std::endl;
    Logger::get() << "Total meshes: " << containter->allMeshes.size() << std::endl;

    containter->finished = true;
}

Transform SceneLoader::parseTransform(const tinygltf::Node& node)
{
    Transform t;

    if (!node.matrix.empty())
    {
        float m[16];
        for (size_t i = 0; i < 16; ++i)
        {
            m[i] = static_cast<float>(node.matrix[i]);
        }
        t = Mat4(m);
        t.emplaceFromMat4();
        return t;
    }
    else if (!node.translation.empty() && !node.rotation.empty())
    {
        t.getQuat().emplace(node.rotation.empty()
                                ? Quat{}
                                : Quat{
                                    std::array{
                                        static_cast<float>(node.rotation[3]),
                                        static_cast<float>(node.rotation[0]),
                                        static_cast<float>(node.rotation[1]),
                                        static_cast<float>(node.rotation[2])
                                    }.data()
                                });

        t.getQuat().value().translate(std::array{
            static_cast<float>(node.translation[0]),
            static_cast<float>(node.translation[1]),
            static_cast<float>(node.translation[2])
        }.data());
        t.getQuat()->normFull();

        if (!node.scale.empty())
        {
            t.scale = Vector3{
                static_cast<float>(node.scale[0]),
                static_cast<float>(node.scale[1]),
                static_cast<float>(node.scale[2])
            };
        }
    } else
    {
        t.emplaceFromMat4();
    }

    return t;
}
