cmake_minimum_required(VERSION 3.0.0)
project(compute-test VERSION 0.1.0)

add_executable(compute-test main.cpp)

# Set to C++17 standard
target_compile_features(compute-test PRIVATE cxx_std_17)

include_directories(${engine_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${engine_LINK_LIBRARIES})
add_definitions(${engine_DEFINES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Copy configuration file
file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/config.json
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

# Copy content
file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/content/
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/content/)