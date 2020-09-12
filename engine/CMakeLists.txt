cmake_minimum_required(VERSION 3.0.0)
project(engine VERSION 0.1.0)

find_package(OpenGL)

# Source files
add_library(engine
    src/core.cpp
    src/engine.cpp
    src/content.cpp
    src/camera.cpp
    src/cameracontroller.cpp
    src/digraph.cpp
    src/forwardrenderer.cpp
    src/geometry.cpp
    src/gui.cpp
    src/halfedge.cpp
    src/halfedgeloader.cpp
    src/input.cpp
    src/lambert.cpp
    src/material.cpp
    src/meshlap.cpp
    src/samplefunction.cpp
    src/sampler.cpp
    src/scene.cpp
    src/shader.cpp
    src/sphericalharmonics.cpp
    src/staticmesh.cpp
    src/texture.cpp
    src/updater.cpp
    src/blit.cpp
    lodepng/lodepng.cpp
    ../nanogui/ext/glad/src/glad.c)

# Preprocessor definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("-- engine building in debug mode.")
    add_definitions(-D_DEBUG)
endif()

add_definitions(
    -DNANOGUI_USE_OPENGL
    -DNANOGUI_GLAD
    -DNANOGUI_SHARED
    -DGLAD_GLAPI_EXPORT)

# Set to C++17 standard
target_compile_features(engine PRIVATE cxx_std_17)

# Include directories
target_include_directories(engine PRIVATE 
    ../nanogui/ext/nanovg/src
    ../assimp/include
    ../gli
    ../engine
    ../nanogui/ext/glfw/include
)

set(${PROJECT_NAME}_INCLUDE_DIRS
    ${OPENGL_INCLUDE_DIRS}
    ../engine/include
    ../glm
    ../nanogui/include
    ../nanogui/ext/glad/include
    ../nanogui/ext/eigen
    ../nanogui/ext/nanovg/src
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
)

set(${PROJECT_NAME}_LINK_LIBRARIES
    ${PROJECT_NAME}
    nanogui
    assimp
    ${OPENGL_LIBRARIES}
    CACHE INTERNAL "${PROJECT_NAME}: Link Libraries" FORCE)

target_include_directories(engine PUBLIC
    ${${PROJECT_NAME}_INCLUDE_DIRS}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
