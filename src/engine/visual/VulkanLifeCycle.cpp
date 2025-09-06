#include "VulkanLifeCycle.h"

#include "ResourceFiller.h"
#include "VulkanContext.h"
#include "../util/Logger.h"
#include "util/BufferUtil.h"

void makeSwapchainDepthBuffer()
{
    VulkanContext::reg(new SwapchainWrap(VulkanContext::surface()->get(), VulkanContext::device()->get(),
                                         VulkanContext::physicalDevice()->get(),
                                         VulkanContext::device()->getIndices(), VulkanContext::window()->get()));
    VulkanContext::swapchainManager()->makeSwapchainImages(VulkanContext::swapchain()->get(),
                                                           VulkanContext::device()->get(),
                                                           VulkanContext::swapchain()->getFormat());
    VulkanContext::reg(new DepthBuffer(*VulkanContext::device()->get(), VulkanContext::physicalDevice()->get(),
                                       VulkanContext::swapchain()->getExtent()));
}

void destroySwapchainDepthBuffer()
{
    vkDeviceWaitIdle(*VulkanContext::device()->get());
    VulkanContext::swapchainManager()->destroySwapchainImages(VulkanContext::device()->get());
    vkDestroySwapchainKHR(*VulkanContext::device()->get(), *VulkanContext::swapchain()->get(), nullptr);
}

void VulkanLifeCycle::showExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    Logger::get() << std::endl << "Available extensions:" << std::endl;
    for (const auto& [extensionName, specVersion] : extensions)
    {
        Logger::get() << extensionName << std::endl;
    }
}

VulkanLifeCycle::VulkanLifeCycle()
{
#ifdef ENGINE_DEBUG_MODE_VALIDATION
    showExtensions();
#endif

    VulkanContext::reg(new SwapchainResourceManager);
    VulkanContext::reg(new InstanceWrap());
    VulkanContext::reg(new WindowWrap());
    VulkanContext::reg(new SurfaceWrap(VulkanContext::instance()->get(), VulkanContext::window()->get()));
    VulkanContext::reg(new PhysicalDeviceWrap(VulkanContext::instance()->get()));
    VulkanContext::reg(new DeviceWrap(VulkanContext::surface()->get(), VulkanContext::physicalDevice()->get(), VulkanContext::physicalDevice()->features));
    VulkanContext::reg(new QueueWrap(VulkanContext::device()->get(), VulkanContext::device()->getIndices()));
    VulkanContext::reg(new CommandPoolWrap(VulkanContext::device()->get(), VulkanContext::device()->getIndices()));
    makeSwapchainDepthBuffer();
    VulkanContext::reg(new ShaderManager());

    // ToDo: scenes must contain shader info?
    VulkanContext::shader()->loadAllFromDirs(Cfg::shaderDirs, VulkanContext::device()->get());
}

VulkanLifeCycle::~VulkanLifeCycle()
{
    Logger::get() << std::endl << "VulkanLifeCycle's destructor called" << std::endl;

    destroySwapchainDepthBuffer();
    VulkanContext::shader()->destroyShaders(VulkanContext::device()->get());
    VulkanContext::commandPool()->destroyPools();

    VkDevice freeDev = *VulkanContext::device()->get();
    VkInstance freeInstance = *VulkanContext::instance()->get();
    VkSurfaceKHR freeSurf = *VulkanContext::surface()->get();
    VulkanContext::clear();
    vkDestroyDevice(freeDev, nullptr);
    vkDestroySurfaceKHR(freeInstance, freeSurf, nullptr);
    vkDestroyInstance(freeInstance, nullptr);

    Logger::get() << "VulkanLifeCycle's destructor finished" << std::endl;
}

void VulkanLifeCycle::fillSceneWithVulkan(Scene* scene)
{
    ResourceFiller::fillAllTextures(VulkanContext::device()->get(), VulkanContext::physicalDevice()->get(),
                                    VulkanContext::commandPool()->getGraphics(), VulkanContext::queue()->getGraphics(),
                                    scene->allTextures);

    ResourceFiller::fillAllMaterialsBeforeDescPool(VulkanContext::device()->get(), scene,
                                                   VulkanContext::physicalDevice()->get(),
                                                   VulkanContext::commandPool()->getGraphics(),
                                                   VulkanContext::queue()->getGraphics());
    ResourceFiller::fillDescriptorPool(scene, VulkanContext::device()->get());
    ResourceFiller::fillAllMaterialsAfterDescPool(scene->allMaterials, scene->descriptorPool.get(),
                                                  VulkanContext::device()->get());
    scene->bufferManager = ResourceFiller::createSceneBufferManager(scene->allMeshes, VulkanContext::device()->get(),
                                                                    VulkanContext::physicalDevice()->get(),
                                                                    VulkanContext::commandPool()->getTransfer(),
                                                                    VulkanContext::queue()->getTransfer());
    scene->descLayout = std::unique_ptr<VkDescriptorSetLayout>(Renderer::makeUboLayout(VulkanContext::device()->get()));
    ResourceFiller::makePipelines(scene, VulkanContext::device()->get(), VulkanContext::physicalDevice()->get(),
                                  VulkanContext::swapchain()->getFormat(),
                                  VulkanContext::shader()->getPipelineShaderStages(Cfg::shaderJsonCfg),
                                  VulkanContext::swapchain()->getExtent(), scene->descLayout.get());
    renderer = std::make_unique<Renderer>(VulkanContext::device()->get(), VulkanContext::physicalDevice()->get(),
                                          VulkanContext::commandPool()->getGraphics(),
                                          scene->renderPass.get(),
                                          VulkanContext::swapchainManager()->getSwapchainImages(),
                                          VulkanContext::depth()->imageView.get(),
                                          VulkanContext::swapchain()->getExtent(), scene->descLayout.get(), scene->gameObjsFlat.size(),
                                          scene->groupedMaterials.size(), scene->allMeshes.size());
}

void VulkanLifeCycle::render(Scene* scene)
{
    if (const auto res = renderer->renderFrame(scene,
                                               VulkanContext::device()->get(),
                                               VulkanContext::queue()->getGraphics(),
                                               VulkanContext::queue()->getPresent(),
                                               VulkanContext::swapchain()->get(),
                                               scene->renderPass.get(),
                                               frameCounter++ % VulkanContext::swapchainManager()->getSwapchainImages().
                                               size(),
                                               VulkanContext::swapchain()->getExtent(),
                                               scene->bufferManager->getVertexBuffer(),
                                               scene->bufferManager->getIndexBuffer()
    ); res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
    {
        vkDeviceWaitIdle(*VulkanContext::device()->get());

        closeScene(scene);
        destroySwapchainDepthBuffer();

        makeSwapchainDepthBuffer();
        fillSceneWithVulkan(scene);
    }
}

void VulkanLifeCycle::closeScene(Scene* scene)
{
    for (const auto& mat: scene->allMaterials)
    {
        if (mat->albedoTexture != nullptr && mat->albedoTexture->name == "DefaultTexture") mat->albedoTexture.reset();
        if (mat->emissiveTexture != nullptr && mat->emissiveTexture->name == "DefaultTexture") mat->emissiveTexture.reset();
        if (mat->normalTexture != nullptr && mat->normalTexture->name == "DefaultTexture") mat->normalTexture.reset();
        if (mat->occlusionTexture != nullptr && mat->occlusionTexture->name == "DefaultTexture") mat->occlusionTexture.reset();
        if (mat->metallicRoughnessTexture != nullptr && mat->metallicRoughnessTexture->name == "DefaultTexture") mat->metallicRoughnessTexture.reset();
    }
    renderer->cleanup(VulkanContext::device()->get(), VulkanContext::commandPool()->getGraphics());
    ResourceFiller::resetVkDependencies(scene, VulkanContext::device()->get());
}

GLFWwindow* VulkanLifeCycle::getWindow()
{
    return VulkanContext::window()->get();
}
