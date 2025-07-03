# RDE_Project/modules/renderer/CMakeLists.txt

# An INTERFACE library has no source files and doesn't produce a binary.
# It's used to group together usage requirements like include directories.
add_library(Renderer)
add_library(RDE::Renderer ALIAS Renderer)

# Anything that links to this library will automatically get its include path.
target_include_directories(Renderer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(Renderer
        PRIVATE
        src/graph/RenderGraph.cpp
)

target_link_libraries(Renderer
        PUBLIC
        RAL
        RDE::Core
        RDE::AssetSystem
        glfw
        glm::glm
)