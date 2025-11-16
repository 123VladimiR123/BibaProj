#include "BibaProj.h"

#include <thread>

#include "Test.h"
#include "engine/scene/SceneLoader.h"
#include "engine/util/Cfg.h"
#include "engine/util/EnumHandler.h"
#include "GLFW/glfw3.h"

std::atomic keepRunning{true};
Quat rot{
    std::array{
        0.99999975f,
        0.0f,
        0.0005f,
        0.0f
    }.data()
};

int main()
{
    testRun();
    VulkanLifeCycle app{};

    auto container = new SceneContainter();
    SceneLoader::loadByDir("donut-4", container);

    auto scene = container->scenes[0].get();
    app.fillSceneWithVulkan(scene);

    std::shared_ptr<GameObject> marker = nullptr;
    std::vector<std::shared_ptr<GameObject>> nonMarkers{};
    for (const auto obj : scene->gameObjsFlat)
        if (obj->markable == MARKABLE_MARKER)
        {
            marker = obj;
        }
        else
        {
            // obj->visible = false;
            nonMarkers.push_back(obj);
        }
    scene->gameObjsFlat = nonMarkers;

    uint32_t counter{};
    while (!glfwWindowShouldClose(app.getWindow()))
    {
        app.render(scene);
        scene->gameObjsRoot[0]->transform->moveAndRotate(rot, std::array{0.0f, 0.00001f, 0.0f}.data());
        if (marker != nullptr && counter % 100 == 0)
        {
            GameObject next = *marker;
            std::shared_ptr<GameObject> nextShared = std::make_shared<GameObject>(next);
            nextShared->transform->setQuat(scene->gameObjsRoot[0]->transform->getQuat().value());
            scene->gameObjsRoot.push_back(nextShared);
            scene->gameObjsFlat.push_back(nextShared);
        }
        glfwPollEvents();
        counter++;
    }

    app.closeScene(container->scenes[0].get());
    return 0;
}
