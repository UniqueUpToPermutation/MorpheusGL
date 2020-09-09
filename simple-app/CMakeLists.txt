cmake_minimum_required(VERSION 3.0.0)
project(simple-app VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(simple-app main.cpp)

include_directories(${engine_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${engine_LINK_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Copy configuration file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json
    ${CMAKE_CURRENT_BINARY_DIR}/config.json
    COPYONLY)