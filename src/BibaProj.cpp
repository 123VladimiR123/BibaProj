#include "BibaProj.h"

#include <thread>

#include "Test.h"
#include "engine/scene/SceneLoader.h"
#include "engine/util/Cfg.h"
#include "GLFW/glfw3.h"

std::atomic keepRunning{true};
Quat rot {std::array{
    0.9999998f,
    0.0005f,
    0.0f,
    0.0f
}.data()};

int main() {
    testRun();
    VulkanLifeCycle app{};

    auto container = new SceneContainter();
    // SceneLoader::loadByDir("boombox", container);
    SceneLoader::loadByDir("donut", container);

    app.fillSceneWithVulkan(container->scenes[0].get());

    while (!glfwWindowShouldClose(app.getWindow())) {
        app.render(container->scenes[0].get());
        container->scenes[0]->gameObjsRoot[0]->transform->moveAndRotate(rot, std::array{0.0f, 0.00003f, 0.0f}.data());
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uint32_t>(1000 * Cfg::deltaTime)));
    }

    app.closeScene(container->scenes[0].get());
    return 0;
}
