#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Cfg
{
public: //toDo variables are overridable getter functions with default value
    static float inline fps = 165.0f;
    static float inline deltaTime = 1.0f / fps;

    static int inline windowWidth = 1200;
    static int inline windowHeight = 675;

    static uint32_t inline selectedGPUID = 0;

    static std::string inline title = "BibkaApp";

    static std::vector<std::string> inline shaderDirs{"shaders/target"};
    static std::string inline shaderJsonCfg{"shaders/cfg.json"}; // toDo remove
    static std::string inline sceneParentDir{"scenes"};
};
