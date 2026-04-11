add_library(owl_core STATIC
    src/Application.cpp
)

target_include_directories(owl_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(owl_core
    PUBLIC
        owl_project_options
)

if(TARGET spdlog::spdlog)
    target_link_libraries(owl_core PUBLIC spdlog::spdlog)
elseif(TARGET spdlog::spdlog_header_only)
    target_link_libraries(owl_core PUBLIC spdlog::spdlog_header_only)
endif()
