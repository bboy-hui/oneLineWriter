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
