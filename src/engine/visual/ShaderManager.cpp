//
// Created by ???????? on 10.08.2025.
//

#include <fstream>
#include <vector>
#include <iostream>
#include "ShaderManager.h"
#include "nlohmann/json.hpp"
#include "../util/Logger.h"
#include "../util/ProjectPath.h"

void ShaderManager::loadOneByPath(const std::filesystem::path& path, VkDevice* device)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) throw std::runtime_error("Failed to open shader file: " + path.string());

    const size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize / sizeof(char));
    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    file.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();

    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(*device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Shader Module: " + path.string());

    Logger::get() << "Shader Module created: " << path << std::endl;
    moduleMap.insert_or_assign(std::move(path.filename().string()), shaderModule);
}

void ShaderManager::loadAllFromDirs(std::vector<std::string>& relativeDirs, VkDevice* device)
{
    const std::string root = ProjectPath::root.string() + "/";
    std::vector<std::filesystem::path> spvFiles;

    for (const std::string& dir : relativeDirs)
    {
        if (!std::filesystem::exists(root + dir)) throw std::runtime_error("Directory does not exits: " + dir);
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root + dir))
            if (entry.is_regular_file() && entry.path().extension() == ".spv")
                spvFiles.push_back(entry.path());
    }

    Logger::get() << std::endl << "Found " << spvFiles.size() << " .spv shader files" << std::endl;
    for (const auto& shaderPath : spvFiles)
        loadOneByPath(shaderPath, device);
}

std::vector<VkPipelineShaderStageCreateInfo> ShaderManager::getPipelineShaderStages(const std::string& pathToCfg)
{
    const std::string cfgPath = ProjectPath::root.string() + "/" + pathToCfg;
    std::ifstream file(cfgPath);

    if (!file.is_open()) throw std::runtime_error("Failed to open shader file: " + cfgPath);

    nlohmann::json data = nlohmann::json::parse(file);
    std::vector<VkPipelineShaderStageCreateInfo> infos;

    for (auto& [k, v] : data.items())
    {
        auto shdIter = moduleMap.find(k);
        if (shdIter == moduleMap.end()) throw std::runtime_error(
            "Shader was defined in config, but shader file wasn't found: " + k);

        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.stage = v["stage"];
        info.module = shdIter->second;
        info.pName = "main";

        infos.push_back(info);
    }

    Logger::get() << std::endl << "Pipeline's stages were created, the total of shaders were binded is " << infos.size()
        << std::endl;
    return infos;
}

void ShaderManager::destroyShaders(VkDevice* device)
{
    for (const auto& val : moduleMap | std::views::values)
        vkDestroyShaderModule(*device, val, nullptr);
}
