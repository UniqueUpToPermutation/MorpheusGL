cmake_minimum_required(VERSION 3.0.0)
project(engine VERSION 0.1.0)

find_package(assimp REQUIRED)

# Include package for embedding shaders in source code
add_subdirectory(shaders/cpp)

# Embed shaders
add_custom_command(
	OUTPUT shader_rc.cpp
	COMMAND embedfile ${CMAKE_CURRENT_SOURCE_DIR}/shaders shader_rc.cpp
	DEPENDS shaders/*)

# Import OpenGL
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
	src/ggx.cpp
	src/brdf.cpp
	src/glslpreprocessor.cpp
	src/skybox.cpp
	src/blend.cpp
	src/spritebatch.cpp

	shader_rc.cpp
	
	lodepng/lodepng.cpp
	stb/src/libstb.c

    ../nanogui/ext/glad/src/glad.c)

# Preprocessor definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("-- engine building in debug mode.")
    add_definitions()

    set(${PROJECT_NAME}_DEFINES
        -D_DEBUG
        -DNANOGUI_USE_OPENGL
        -DNANOGUI_GLAD
        -DNANOGUI_SHARED
        -DGLAD_GLAPI_EXPORT
        CACHE INTERNAL "${PROJECT_NAME}: Preprocessor Defines" FORCE)
else()
    set(${PROJECT_NAME}_DEFINES
    -DNANOGUI_USE_OPENGL
    -DNANOGUI_GLAD
    -DNANOGUI_SHARED
    -DGLAD_GLAPI_EXPORT
    CACHE INTERNAL "${PROJECT_NAME}: Preprocessor Defines" FORCE)
endif()

add_definitions(${${PROJECT_NAME}_DEFINES})

# Set to C++17 standard
target_compile_features(engine PRIVATE cxx_std_17)

# Include directories
target_include_directories(engine PRIVATE 
    ../nanogui/ext/nanovg/src
    ../gli
    ../engine
    ../incbin
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

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("Compiling for Darwin!")
    set(${PROJECT_NAME}_LINK_LIBRARIES
        ${PROJECT_NAME}
        nanogui
        ${ASSIMP_LIBRARIES}
        glfw3
        ${OPENGL_LIBRARIES}
        "-framework Cocoa"
        "-framework IOKit"
        CACHE INTERNAL "${PROJECT_NAME}: Link Libraries" FORCE)

    target_link_libraries(engine 
        ${OPENGL_LIBRARIES}
        glfw3
        "-framework Cocoa"
        "-framework IOKit"
        ${ASSIMP_LIBRARIES}
        nanogui)
else()
    set(${PROJECT_NAME}_LINK_LIBRARIES
        ${PROJECT_NAME}
        nanogui
        ${ASSIMP_LIBRARIES}
        glfw3
        ${OPENGL_LIBRARIES}
        CACHE INTERNAL "${PROJECT_NAME}: Link Libraries" FORCE)
endif()

target_include_directories(engine PUBLIC
	stb/include
    ${${PROJECT_NAME}_INCLUDE_DIRS}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
