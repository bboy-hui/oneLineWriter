add_library(owl_device STATIC
    src/GrblController.cpp
)

target_include_directories(owl_device
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(owl_device
    PUBLIC
        owl_project_options
)
