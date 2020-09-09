cmake_minimum_required(VERSION 3.0.0)
project(Morpheus VERSION 0.1.0)

include(CTest)
enable_testing()

option(BUILD_ENGINE "Enable building engine" ON)
option(BUILD_ASSIMP "Enable building assimp" ON)
option(BUILD_NANOGUI "Enable building nanogui" ON)
option(BUILD_SIMPLE_APP "Enable building simple app" ON)

if (BUILD_NANOGUI)
    add_subdirectory(nanogui)
endif()

if (BUILD_ASSIMP)
    add_subdirectory(assimp)
endif()

if(BUILD_ENGINE)
    add_subdirectory(engine)
endif()

if(BUILD_SIMPLE_APP)
    add_subdirectory(simple-app)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
