//
// Created by ???????? on 10.08.2025.
//

#ifndef BIBAPROJ_SHADERMANAGER_H
#define BIBAPROJ_SHADERMANAGER_H


#include <vulkan/vulkan_core.h>
#include <string>
#include <filesystem>
#include <unordered_map>

class ShaderManager {
    std::unordered_map<std::string, VkShaderModule> moduleMap;

public:
    ShaderManager(const ShaderManager &) = delete;

    ShaderManager &operator=(const ShaderManager &) = delete;

    ShaderManager() = default;

    void loadOneByPath(const std::filesystem::path &, VkDevice *);

    void loadAllFromDirs(std::vector<std::string> &, VkDevice *);

    void destroyShaders(VkDevice *);

    std::vector<VkPipelineShaderStageCreateInfo> getPipelineShaderStages(const std::string& pathToCfg);
};


#endif //BIBAPROJ_SHADERMANAGER_H
