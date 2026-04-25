cmake_minimum_required(VERSION 3.24)

# vcpkg 工具链
set(CMAKE_TOOLCHAIN_FILE "D:/software/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain")

project(
    OneLineWriter
    VERSION 0.1.0
    DESCRIPTION "GRBL upper-computer writing and laser workflow"
    LANGUAGES CXX
)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 输出目录配置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

option(OLW_BUILD_DESKTOP "Build desktop executable" ON)
option(BUILD_TEST "Build test executable" OFF)

# 核心静态库
add_subdirectory(src)

# 带界面的可执行文件
if(OLW_BUILD_DESKTOP)
    add_subdirectory(apps)
endif()

# 测试可执行文件
if(BUILD_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()