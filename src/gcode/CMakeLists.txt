add_library(owl_gcode STATIC
    src/GCodeGenerator.cpp
    src/GCodeIo.cpp
    src/TextStrokePlanner.cpp
)

target_include_directories(owl_gcode
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(owl_gcode
    PUBLIC
        owl_project_options
)
