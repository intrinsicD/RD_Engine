# RDE_Project/modules/renderer/CMakeLists.txt

# An INTERFACE library has no source files and doesn't produce a binary.
# It's used to group together usage requirements like include directories.
add_library(Renderer)
add_library(RDE::Renderer ALIAS Renderer)

# Anything that links to this library will automatically get its include path.
target_include_directories(Renderer
        PUBLIC
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${glm_SOURCE_DIR}
)

target_sources(Renderer
        PRIVATE
        src/EditorCamera.cpp
        src/Mesh.cpp
        src/Renderer.cpp
        src/Renderer2D.cpp
        src/Renderer3D.cpp
        src/RenderCommand.cpp
        src/PerspectiveCamera.cpp
        src/OrthographicCamera.cpp
        src/OrthographicCameraController.cpp
)

target_link_libraries(Renderer
        PUBLIC
        RDE::Core
)