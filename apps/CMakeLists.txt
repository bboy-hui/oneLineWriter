find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(VULKAN_SDK "C:/VulkanSDK/1.3.280.0")

add_executable(owl_desktop
    main.cpp
    FontManager.cpp
    ${CMAKE_SOURCE_DIR}/3rd/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/3rd/imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(owl_desktop PRIVATE
    ${CMAKE_SOURCE_DIR}/3rd/imgui
    ${CMAKE_SOURCE_DIR}/3rd/imgui/backends
    D:/software/vcpkg/installed/x64-windows/include
    ${VULKAN_SDK}/Include
)

target_link_directories(owl_desktop PRIVATE
    ${VULKAN_SDK}/Lib
)

target_link_libraries(owl_desktop PRIVATE
    glfw
    spdlog::spdlog
    D:/software/vcpkg/installed/x64-windows/lib/imgui.lib
    vulkan-1
    user32
    gdi32
)

add_custom_command(TARGET owl_desktop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/software/vcpkg/installed/x64-windows/bin/glfw3.dll"
        $<TARGET_FILE_DIR:owl_desktop>
)
