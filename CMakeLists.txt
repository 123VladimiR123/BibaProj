cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(BibaProj LANGUAGES CXX)

if (MSVC AND POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif ()

set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)

add_subdirectory(src/engine/thirdparty/glfw-3.4)
add_subdirectory(src/engine/thirdparty/json-3.12.0)

set(SOURCES
        src/BibaProj.cpp
        src/engine/maths/Quat.cpp
        src/engine/maths/Util.cpp
        src/engine/maths/DualQuat.cpp
        src/engine/maths/Mat4.cpp
        src/engine/gameobj/nested/Transform.cpp
        src/engine/visual/SwapchainResourceManager.cpp
        src/engine/visual/VulkanLifeCycle.cpp
        src/engine/visual/DeviceWrap.cpp
        src/engine/visual/SurfaceWrap.cpp
        src/engine/visual/SwapchainWrap.cpp
        src/engine/visual/InstanceWrap.cpp
        src/engine/visual/PhysicalDeviceWrap.cpp
        src/engine/visual/WindowWrap.cpp
        src/engine/visual/VulkanContext.cpp
        src/engine/visual/QueueWrap.cpp src/engine/util/Logger.cpp
        src/engine/visual/ShaderManager.cpp
        src/engine/gameobj/nested/MeshPrimitive.h
        src/engine/gameobj/GameObject.h
        src/engine/gameobj/nested/Material.h
        src/engine/gameobj/nested/Texture.h
        src/engine/scene/Scene.cpp
        src/engine/scene/Scene.h
        src/engine/scene/SceneLoader.cpp
        src/engine/scene/SceneLoader.h
        src/engine/util/ProjectPath.cpp
        src/engine/scene/SceneContainer.h
        src/engine/maths/Tangent4.h
        src/engine/maths/Vector4.h
        src/engine/util/Cfg.h
        src/engine/gameobj/camera/Camera.h
        src/engine/gameobj/camera/PerspectiveCamera.h
        src/engine/gameobj/camera/OrthographicCamera.h
        src/engine/visual/ResourceFiller.cpp
        src/engine/visual/ResourceFiller.h
        src/engine/visual/CommandPoolWrap.cpp
        src/engine/visual/CommandPoolWrap.h
        src/engine/visual/MeshBufferManager.cpp
        src/engine/visual/MeshBufferManager.h
        src/engine/visual/util/BufferUtil.cpp
        src/engine/visual/util/BufferUtil.h
        src/engine/visual/Renderer.cpp
        src/engine/visual/Renderer.h
        src/engine/visual/util/UniformBufferObject.h
        src/engine/visual/DepthBuffer.cpp
        src/engine/visual/DepthBuffer.h
        src/Test.h
)

add_executable(BibaProj ${SOURCES})

#debug output
#add_compile_definitions(ENGINE_DEBUG_MODE_VALIDATION)
add_compile_definitions(ENGINE_DEBUG_MODE_LOGGING)

find_package(Vulkan REQUIRED)

target_link_libraries(BibaProj PRIVATE glfw Vulkan::Vulkan)
target_link_libraries(BibaProj PRIVATE nlohmann_json::nlohmann_json)

target_include_directories(BibaProj PRIVATE src/engine/thirdparty/tinygltf-2.9.6)

if (WIN32)
    target_compile_definitions(BibaProj PRIVATE VK_USE_PLATFORM_WIN32_KHR)
endif ()

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
    target_compile_features(BibaProj PRIVATE cxx_std_20)
endif ()

target_compile_options(BibaProj PRIVATE -msse4.1)