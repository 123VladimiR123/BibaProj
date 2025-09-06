//
// Created by Владимир on 08.08.2025.
//

#include <stdexcept>
#include <iostream>
#include "WindowWrap.h"
#include "../util/Cfg.h"
#include "../util/Logger.h"

WindowWrap::WindowWrap() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfWwindow = glfwCreateWindow(Cfg::windowWidth, Cfg::windowHeight, Cfg::title.data(), nullptr, nullptr);
    if (glfWwindow == nullptr)
        throw std::runtime_error("Window creation failed");
    Logger::get() << "GLFW Window created" << std::endl;
}

