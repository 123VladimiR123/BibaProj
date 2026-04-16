#include "BibaProj.h"

#include <random>
#include <thread>

#include "Test.h"
#include "engine/scene/SceneLoader.h"
#include "engine/util/Cfg.h"
#include "engine/util/EnumHandler.h"
#include "GLFW/glfw3.h"

std::atomic keepRunning{true};
Quat rot{
    std::array{
        0.999999f,
        0.0f,
        0.0008f,
        0.0f
    }.data()
};

int main()
{
    VulkanLifeCycle app{};

    auto container = new SceneContainter();
    SceneLoader::loadByDir("boombox", container);

    auto scene = container->scenes[0].get();
    app.fillSceneWithVulkan(scene);

    while (!glfwWindowShouldClose(app.getWindow()))
    {
        app.render(scene);
        glfwPollEvents();
        for (const auto& obj : scene->gameObjsRoot)
            obj->transform->rotate(rot);
    }

    app.closeScene(container->scenes[0].get());
    return 0;
}
