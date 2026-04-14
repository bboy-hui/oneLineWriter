set(OWL_IMGUI_DIR "${CMAKE_SOURCE_DIR}/3rd/imgui")

if(NOT EXISTS "${OWL_IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR "ImGui sources not found at ${OWL_IMGUI_DIR}")
endif()

add_library(owl_imgui STATIC
    ${OWL_IMGUI_DIR}/imgui.cpp
    ${OWL_IMGUI_DIR}/imgui_draw.cpp
    ${OWL_IMGUI_DIR}/imgui_tables.cpp
    ${OWL_IMGUI_DIR}/imgui_widgets.cpp
    ${OWL_IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

if(WIN32)
    target_sources(owl_imgui PRIVATE
        ${OWL_IMGUI_DIR}/backends/imgui_impl_win32.cpp
    )
endif()

target_include_directories(owl_imgui
    PUBLIC
        ${OWL_IMGUI_DIR}
        ${OWL_IMGUI_DIR}/backends
)

target_link_libraries(owl_imgui
    PUBLIC
        owl_project_options
)

add_executable(owl_desktop
    src/DesktopWorkbench.cpp
    src/Viewport3DWindow.cpp
    src/Viewport3DWindowVulkan.cpp
    src/main.cpp
)

target_link_libraries(owl_desktop
    PRIVATE
        owl_project_options
        owl_core
        owl_render
        owl_gcode
        owl_device
        owl_imgui
)

if(WIN32)
    target_link_libraries(owl_desktop PRIVATE opengl32)
endif()

if(WIN32)
    find_package(Vulkan REQUIRED)
    target_link_libraries(owl_desktop PRIVATE Vulkan::Vulkan)

    find_program(OWL_GLSLANG_VALIDATOR
        NAMES glslangValidator glslangValidator.exe
        HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VULKAN_SDK}/Bin32"
    )

    if(NOT OWL_GLSLANG_VALIDATOR)
        message(FATAL_ERROR "glslangValidator not found. Install Vulkan SDK or set VULKAN_SDK.")
    endif()

    set(OWL_VIEWPORT_SHADER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    set(OWL_VIEWPORT_SHADER_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

    set(OWL_VIEWPORT_VERT_SPV "${OWL_VIEWPORT_SHADER_BIN_DIR}/viewport3d.vert.spv")
    set(OWL_VIEWPORT_FRAG_SPV "${OWL_VIEWPORT_SHADER_BIN_DIR}/viewport3d.frag.spv")

    add_custom_command(
        OUTPUT "${OWL_VIEWPORT_VERT_SPV}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${OWL_VIEWPORT_SHADER_BIN_DIR}"
        COMMAND "${OWL_GLSLANG_VALIDATOR}" -V "${OWL_VIEWPORT_SHADER_SRC_DIR}/viewport3d.vert" -o "${OWL_VIEWPORT_VERT_SPV}"
        DEPENDS "${OWL_VIEWPORT_SHADER_SRC_DIR}/viewport3d.vert"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${OWL_VIEWPORT_FRAG_SPV}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${OWL_VIEWPORT_SHADER_BIN_DIR}"
        COMMAND "${OWL_GLSLANG_VALIDATOR}" -V "${OWL_VIEWPORT_SHADER_SRC_DIR}/viewport3d.frag" -o "${OWL_VIEWPORT_FRAG_SPV}"
        DEPENDS "${OWL_VIEWPORT_SHADER_SRC_DIR}/viewport3d.frag"
        VERBATIM
    )

    add_custom_target(owl_desktop_shaders
        DEPENDS
            "${OWL_VIEWPORT_VERT_SPV}"
            "${OWL_VIEWPORT_FRAG_SPV}"
    )

    add_dependencies(owl_desktop owl_desktop_shaders)

    add_custom_command(TARGET owl_desktop POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:owl_desktop>/shaders"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${OWL_VIEWPORT_VERT_SPV}"
            "$<TARGET_FILE_DIR:owl_desktop>/shaders/viewport3d.vert.spv"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${OWL_VIEWPORT_FRAG_SPV}"
            "$<TARGET_FILE_DIR:owl_desktop>/shaders/viewport3d.frag.spv"
        VERBATIM
    )
endif()
