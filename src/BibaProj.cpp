#include "BibaProj.h"

#include <thread>

#include "Test.h"
#include "engine/scene/SceneLoader.h"
#include "GLFW/glfw3.h"

std::atomic keepRunning{true};
Quat rot {std::array{
    0.9999998f,
    0.0f,
    0.005f,
    0.0f
}.data()};

void rotationThread(Scene* scene, Quat rot) {
    while (keepRunning) {
        // scene->gameObjsRoot[0]->transform->rotate(rot);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    testRun();
    VulkanLifeCycle app{};

    auto container = new SceneContainter();
    SceneLoader::loadByDir("donut", container);

    app.fillSceneWithVulkan(container->scenes[0].get());

    std::thread rotThread(rotationThread, container->scenes[0].get(), rot);

    while (!glfwWindowShouldClose(app.getWindow())) {
        app.render(container->scenes[0].get());
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    keepRunning = false;
    rotThread.join();

    app.closeScene(container->scenes[0].get());
    return 0;
}
