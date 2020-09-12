cmake_minimum_required(VERSION 3.0.0)
project(Morpheus VERSION 0.1.0)

include(CTest)
enable_testing()

option(BUILD_ENGINE "Enable building engine" ON)
option(BUILD_ASSIMP "Enable building assimp" ON)
option(BUILD_NANOGUI "Enable building nanogui" ON)
option(BUILD_SIMPLE_APP "Enable building simple app" ON)
option(BUILD_PBR "Enable building PBR demo" ON)
option(BUILD_LAPVIEWER "Enable builing Laplacian Eigenvector viewer" ON)
option(BUILD_BLITTEST "Enable building blit test" ON)

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

if(BUILD_PBR)
    add_subdirectory(pbr)
endif()

if(BUILD_LAPVIEWER)
    add_subdirectory(lapviewer)
endif()

if(BUILD_BLITTEST)
    add_subdirectory(blit-test)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
