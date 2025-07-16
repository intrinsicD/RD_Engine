add_library(Renderer)
add_library(RDE::Renderer ALIAS Renderer)

target_include_directories(Renderer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(Renderer
        PRIVATE
        src/Renderer.cpp
        src/ShaderReflector.cpp
        src/PipelineCache.cpp
)

target_link_libraries(Renderer
        PUBLIC
        RDE::Core
        RDE::AssetSystem
        RDE::Platform::Vulkan
)