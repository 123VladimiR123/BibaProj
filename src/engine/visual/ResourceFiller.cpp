//
// Created by admin on 8/27/2025.
//

#include "ResourceFiller.h"

#include <bitset>
#include <memory>

#include "../scene/Scene.h"
#include "../util/Logger.h"
#include "util/BufferUtil.h"

std::shared_ptr<Texture> ResourceFiller::defaultTexture{};

void transitionImageLayout(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,
                           VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

std::unique_ptr<MeshBufferManager> ResourceFiller::createSceneBufferManager(
    std::vector<std::shared_ptr<MeshPrimitive>>& allMeshes,
    VkDevice* device, VkPhysicalDevice* physicalDevice, VkCommandPool* commandPool, VkQueue* transferQueue)
{
    return std::make_unique<MeshBufferManager>(allMeshes, device, physicalDevice, commandPool, transferQueue,
                                               BufferUtil::createBuffer, BufferUtil::copyBuffer);
}

void ResourceFiller::fillTexture(VkDevice* device, VkPhysicalDevice* physicalDevice, VkCommandPool* commandPool,
                                 VkQueue* graphicsQueue, Texture& texture)
{
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    BufferUtil::createBuffer(
        *device,
        *physicalDevice,
        texture.width * texture.height * texture.channels,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    void* data;
    vkMapMemory(*device, stagingBufferMemory, 0, texture.pixels.size(), 0, &data);
    memcpy(data, texture.pixels.data(), texture.pixels.size());
    vkUnmapMemory(*device, stagingBufferMemory);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texture.width;
    imageInfo.extent.height = texture.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = texture.format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(*device, &imageInfo, nullptr, &texture.image) != VK_SUCCESS)
    {
        Logger::get() << "Unable to create vkImage for Texture" << std::endl;
        texture.image = VK_NULL_HANDLE;
        return;
    }

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(*device, texture.image, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;

    uint32_t memTypeIndex = BufferUtil::findMemoryType(*physicalDevice, memReq.memoryTypeBits,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memTypeIndex;

    if (vkAllocateMemory(*device, &allocInfo, nullptr, &texture.memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to allocate memory for Texture");
    }

    vkBindImageMemory(*device, texture.image, texture.memory, 0);

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(*device, *commandPool);

    transitionImageLayout(commandBuffer, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {texture.width, texture.height, 1};

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    transitionImageLayout(commandBuffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    endSingleTimeCommands(*device, *commandPool, *graphicsQueue, commandBuffer);

    vkDestroyBuffer(*device, stagingBuffer, nullptr);
    vkFreeMemory(*device, stagingBufferMemory, nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = texture.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(*device, &viewInfo, nullptr, &texture.imageView) != VK_SUCCESS)
    {
        Logger::get() << "Unable to create vkImageView for Texture" << std::endl;
        texture.imageView = VK_NULL_HANDLE;
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = texture.samplerInfo.magFilter;
    samplerInfo.minFilter = texture.samplerInfo.minFilter;
    samplerInfo.addressModeU = texture.samplerInfo.addressModeU;
    samplerInfo.addressModeV = texture.samplerInfo.addressModeV;
    samplerInfo.addressModeW = texture.samplerInfo.addressModeW;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;

    if (vkCreateSampler(*device, &samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create VkSampler for Texture obj");
    }
}

void ResourceFiller::fillAllTextures(VkDevice* device, VkPhysicalDevice* physicalDevice, VkCommandPool* commandPool,
                                     VkQueue* graphicsQueue,
                                     std::vector<std::shared_ptr<Texture>>& textures)
{
    for (auto& text : textures)
        fillTexture(device, physicalDevice, commandPool, graphicsQueue, *text);
    Logger::get() << std::endl << "Vulkan resources created for scene's textures" << std::endl;
}

void ResourceFiller::fillAllMaterialsBeforeDescPool(VkDevice* device, Scene* scene, VkPhysicalDevice* physicalDevice,
                                                    VkCommandPool* commandPool, VkQueue* graphicsQueue)
{
    std::unordered_map<uint_fast8_t, std::vector<std::shared_ptr<Material>>> materialsByTextures;
    std::shared_ptr<Texture> pDefText = std::shared_ptr<Texture>(
        makeDefaultTexture(device, physicalDevice, commandPool, graphicsQueue));

    for (const auto& mat : scene->allMaterials)
    {
        if (mat->albedoTexture == nullptr)
            mat->albedoTexture = pDefText;
        if (mat->normalTexture == nullptr)
            mat->normalTexture = pDefText;
        if (mat->occlusionTexture == nullptr)
            mat->occlusionTexture = pDefText;
        if (mat->metallicRoughnessTexture == nullptr)
            mat->metallicRoughnessTexture = pDefText;
        if (mat->emissiveTexture == nullptr)
            mat->emissiveTexture = pDefText;
    }

    for (const auto& mat : scene->allMaterials)
    {
        uint_fast8_t curr = 0;
        if (mat->albedoTexture != nullptr)
            curr |= 1;
        if (mat->metallicRoughnessTexture != nullptr)
            curr |= 2;
        if (mat->normalTexture != nullptr)
            curr |= 4;
        if (mat->occlusionTexture != nullptr)
            curr |= 8;
        if (mat->emissiveTexture != nullptr)
            curr |= 16;

        materialsByTextures[curr].push_back(mat);
    }

    for (const auto& pair : materialsByTextures)
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.push_back(uboLayoutBinding);

        for (uint_fast8_t i = 0; i < 5; i++)
        {
            if ((pair.first & 1 << i) != 0)
            {
                VkDescriptorSetLayoutBinding textureBinding{};
                textureBinding.binding = static_cast<uint32_t>(i + 1);
                textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                textureBinding.descriptorCount = 1;
                textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                textureBinding.pImmutableSamplers = nullptr;

                bindings.push_back(textureBinding);
            }
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        const auto deleter = [device](const VkDescriptorSetLayout* layout)
        {
            vkDestroyDescriptorSetLayout(*device, *layout, nullptr);
        };

        std::shared_ptr<VkDescriptorSetLayout> layout(new VkDescriptorSetLayout, deleter);
        if (vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, layout.get()) != VK_SUCCESS)
            throw std::runtime_error(
                "Unable to create VkDescriptorSetLayout for materials set with key: 0b" + std::bitset<5>(pair.first).
                to_string());

        for (const auto& mat : pair.second)
        {
            mat->descriptorLayout = layout;
        }
    }

    scene->groupedMaterials = materialsByTextures;
    Logger::get() << "Vulkan resources created for scene's materials" << std::endl;
}

void ResourceFiller::fillDescriptorPool(Scene* scene, VkDevice* device)
{
    if (scene == nullptr)
        throw std::runtime_error("Scene's pointer is nullptr");
    if (scene->allMaterials.empty())
        throw std::runtime_error("No materials. Unable to create descriptor pool");

    std::vector<VkDescriptorPoolSize> poolSizes(1);
    constexpr float x = 1.2f;
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(static_cast<float>(scene->allMaterials.size()) * x);

    VkDescriptorPoolSize size;
    size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    size.descriptorCount = static_cast<uint32_t>(static_cast<float>(scene->allMaterials.size() * 5) * x);
    poolSizes.push_back(size);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(static_cast<float>(scene->allMaterials.size()) * x);

    scene->descriptorPool = std::unique_ptr<VkDescriptorPool, std::function<void(VkDescriptorPool*)>>(
        new VkDescriptorPool(), [device](const VkDescriptorPool* pool)
        {
            vkDestroyDescriptorPool(*device, *pool, nullptr);
        });

    if (vkCreateDescriptorPool(*device, &poolInfo, nullptr, scene->descriptorPool.get()) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create descriptor pool");
    }

    Logger::get() << "Descriptor pool created for scene with x = " << x << std::endl;
}

std::shared_ptr<Texture> ResourceFiller::makeDefaultTexture(VkDevice* device, VkPhysicalDevice* physicalDevice,
                                                            VkCommandPool* commandPool, VkQueue* graphicsQueue)
{
    defaultTexture = std::make_shared<Texture>();

    defaultTexture->name = "DefaultTexture";
    defaultTexture->width = 1;
    defaultTexture->height = 1;
    defaultTexture->channels = 4;
    defaultTexture->format = VK_FORMAT_R8G8B8A8_UNORM;
    defaultTexture->pixels = {255, 255, 255, 255};

    fillTexture(device, physicalDevice, commandPool, graphicsQueue, *defaultTexture);

    return defaultTexture;
}

void ResourceFiller::fillAllMaterialsAfterDescPool(std::vector<std::shared_ptr<Material>>& materials,
                                                   VkDescriptorPool* pool, VkDevice* device)
{
    for (const auto& mat : materials)
    {
        if (mat == nullptr) continue;
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = *pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = mat->descriptorLayout.get();

        mat->descriptor = std::make_shared<VkDescriptorSet>();
        if (vkAllocateDescriptorSets(*device, &allocInfo, mat->descriptor.get()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate descriptor set!");
        }

        std::array<VkDescriptorImageInfo, 5> imageInfos{};
        std::array<VkWriteDescriptorSet, 5> materialWrites{};

        auto addWrite = [&](const Texture& text, const uint32_t order) {
            size_t index = order - 1;

            imageInfos[index].sampler = text.sampler;
            imageInfos[index].imageView = text.imageView;
            imageInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            materialWrites[index].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            materialWrites[index].dstSet = *mat->descriptor;
            materialWrites[index].dstBinding = order;
            materialWrites[index].descriptorCount = 1;
            materialWrites[index].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            materialWrites[index].pImageInfo = &imageInfos[index];
        };

        addWrite(*mat->albedoTexture, 1);
        addWrite(*mat->metallicRoughnessTexture, 2);
        addWrite(*mat->normalTexture, 3);
        addWrite(*mat->occlusionTexture, 4);
        addWrite(*mat->emissiveTexture, 5);

        vkUpdateDescriptorSets(*device, materialWrites.size(), materialWrites.data(), 0, nullptr);
    }

    Logger::get() << "Textures were described by materials' descriptor sets" << std::endl;
}

void ResourceFiller::resetVkDependencies(Scene* scene, VkDevice *device)
{
    vkDeviceWaitIdle(*device);
    vkDestroyImageView(*device, defaultTexture->imageView, nullptr);
    vkDestroyImage(*device, defaultTexture->image, nullptr);
    vkFreeMemory(*device, defaultTexture->memory, nullptr);
    vkDestroySampler(*device, defaultTexture->sampler, nullptr);
    for (const auto& tex: scene->allTextures)
    {
        vkDestroyImageView(*device, tex->imageView, nullptr);
        vkDestroyImage(*device, tex->image, nullptr);
        vkFreeMemory(*device, tex->memory, nullptr);
        vkDestroySampler(*device, tex->sampler, nullptr);
    }
    scene->renderPass.reset(nullptr);
    scene->descriptorPool.reset(nullptr);
    vkDestroyDescriptorSetLayout(*device, *scene->descLayout, nullptr);
    for (const auto& mat : scene->allMaterials)
    {
        mat->descriptorLayout.reset();
        mat->descriptor.reset();
        mat->pipelineLayout.reset();
        mat->pipeline.reset();
    }
    if (scene->bufferManager != nullptr)
    {
        scene->bufferManager.reset(nullptr);
    }
    Logger::get() << "Scene's vk resources released" << std::endl;
}

VkFormat findDepthFormat(const VkPhysicalDevice* physicalDevice)
{
    return BufferUtil::findSupportedFormat(
        physicalDevice,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void ResourceFiller::makePipelines(Scene* scene, VkDevice* device, VkPhysicalDevice* physicalDevice,
                                   VkSurfaceFormatKHR* format,
                                   std::vector<VkPipelineShaderStageCreateInfo> shaderStages, VkExtent2D* extent,
                                   VkDescriptorSetLayout* uboLayout)
{
    if (scene == nullptr)
        throw std::runtime_error("Scene is nullptr");
    const auto renderPassDeleter = [device](const VkRenderPass* pass)
    {
        vkDestroyRenderPass(*device, *pass, nullptr);
    };
    scene->renderPass = std::unique_ptr<VkRenderPass, std::function<void(VkRenderPass*)>>(
        new VkRenderPass(), renderPassDeleter);

    // toDo: all graphics settings to configs
    VkVertexInputBindingDescription bindingDesc;
    std::array<VkVertexInputAttributeDescription, 5> attributeDesc{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};

    VkRenderPassCreateInfo renderPassInfo{};
    VkSubpassDependency dependency{};
    VkSubpassDescription subpass{};
    VkAttachmentReference depthAttachmentRef{};
    VkAttachmentReference colorAttachmentRef{};
    VkAttachmentDescription depthAttachment{};
    VkAttachmentDescription colorAttachment{};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkViewport viewport{};
    VkRect2D scissor{};
    VkPipelineViewportStateCreateInfo viewportState{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineMultisampleStateCreateInfo multisampling{};
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlending{};

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent->width);
    viewport.height = static_cast<float>(extent->height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = {0, 0};
    scissor.extent = *extent;

    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_TRUE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //toDo define
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //  VK_FRONT_FACE_COUNTER_CLOCKWISE
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; //wtf
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(Vertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributeDesc[0].binding = 0;
    attributeDesc[0].location = 0;
    attributeDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDesc[0].offset = offsetof(Vertex, position);

    attributeDesc[1].binding = 0;
    attributeDesc[1].location = 1;
    attributeDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDesc[1].offset = offsetof(Vertex, normal);

    attributeDesc[2].binding = 0;
    attributeDesc[2].location = 2;
    attributeDesc[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDesc[2].offset = offsetof(Vertex, tangent);

    attributeDesc[3].binding = 0;
    attributeDesc[3].location = 3;
    attributeDesc[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDesc[3].offset = offsetof(Vertex, bitangent);

    attributeDesc[4].binding = 0;
    attributeDesc[4].location = 4;
    attributeDesc[4].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDesc[4].offset = offsetof(Vertex, uv);

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesc.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDesc.data();

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    colorAttachment.format = format->format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    depthAttachment.format = findDepthFormat(physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array attachments = {colorAttachment, depthAttachment};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(*device, &renderPassInfo, nullptr, scene->renderPass.get()) != VK_SUCCESS)
            throw std::runtime_error("Failed to create render pass");

    const auto layoutDeleter = [device](const VkPipelineLayout* layout)
    {
        vkDestroyPipelineLayout(*device, *layout, nullptr);
    };

    const auto pipelineDeleter = [device](const VkPipeline* pipeline)
    {
        vkDestroyPipeline(*device, *pipeline, nullptr);
    };

    for (const auto& mat : scene->groupedMaterials)
    {
        if (mat.second.empty())
            throw std::runtime_error("Material group is empty, but bitmask exists");

        std::array descriptorLayouts = {
            *uboLayout,
            *mat.second[0]->descriptorLayout
        };

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(Mat4);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 2; // as ubo + own layout
        pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        auto pipeLayout = std::shared_ptr<VkPipelineLayout>(new VkPipelineLayout(), layoutDeleter);
        if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, pipeLayout.get()) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout");

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = *pipeLayout;
        pipelineInfo.renderPass = *scene->renderPass;
        pipelineInfo.subpass = 0;

        auto pipeline = std::shared_ptr<VkPipeline>(new VkPipeline(), pipelineDeleter);
        if (vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, pipeline.get()) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline");

        for (const auto& matInGroup : mat.second)
        {
            matInGroup->pipelineLayout = pipeLayout;
            matInGroup->pipeline = pipeline;
        }
    }

    Logger::get() << "Shared pipelines created" << std::endl;
}
