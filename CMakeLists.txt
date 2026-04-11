cmake_minimum_required(VERSION 3.24)

project(
    OneLineWriter
    VERSION 0.1.0
    DESCRIPTION "GRBL upper-computer writing and laser workflow"
    LANGUAGES CXX
)

include(cmake/CompilerOptions.cmake)

option(OWL_BUILD_DESKTOP "Build desktop executable" ON)
option(OWL_ENABLE_VULKAN_BACKEND "Enable Vulkan renderer backend support" ON)
option(OWL_ENABLE_OPENGL_BACKEND "Enable OpenGL renderer backend support" ON)

if(EXISTS "${CMAKE_SOURCE_DIR}/3rd/spdlog/CMakeLists.txt")
    set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    add_subdirectory(3rd/spdlog)
endif()

add_library(owl_project_options INTERFACE)
owl_apply_compiler_options(owl_project_options)

add_subdirectory(src/core)
add_subdirectory(src/render)
add_subdirectory(src/gcode)
add_subdirectory(src/device)

if(OWL_BUILD_DESKTOP)
    add_subdirectory(apps/desktop)
endif()
