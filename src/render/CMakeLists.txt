add_library(owl_render STATIC
    src/OpenGLRenderBackend.cpp
    src/RenderBackendFactory.cpp
    src/RenderService.cpp
    src/VulkanRenderBackend.cpp
)

target_include_directories(owl_render
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(owl_render
    PUBLIC
        owl_project_options
)

target_compile_definitions(owl_render
    PUBLIC
        $<$<BOOL:${OWL_ENABLE_OPENGL_BACKEND}>:OWL_ENABLE_OPENGL_BACKEND=1>
        $<$<BOOL:${OWL_ENABLE_VULKAN_BACKEND}>:OWL_ENABLE_VULKAN_BACKEND=1>
)
