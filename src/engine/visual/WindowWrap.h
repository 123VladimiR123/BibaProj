//
// Created by ???????? on 08.08.2025.
//

#ifndef BIBAPROJ_WINDOWWRAP_H
#define BIBAPROJ_WINDOWWRAP_H

#include "GLFW/glfw3.h"

class WindowWrap {
    GLFWwindow *glfWwindow;
public:
    WindowWrap(const WindowWrap &) = delete;

    WindowWrap &operator=(const WindowWrap &) = delete;

    WindowWrap();

    [[nodiscard]] GLFWwindow *get() const { return glfWwindow; }
};


#endif //BIBAPROJ_WINDOWWRAP_H
