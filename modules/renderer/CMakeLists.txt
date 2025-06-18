# RDE_Project/modules/renderer/CMakeLists.txt

# An INTERFACE library has no source files and doesn't produce a binary.
# It's used to group together usage requirements like include directories.
add_library(Renderer)
add_library(RDE::Renderer ALIAS Renderer)

# Anything that links to this library will automatically get its include path.
target_include_directories(Renderer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${glm_SOURCE_DIR}
)

target_sources(Renderer
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer2D.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderCommand.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/OrthographicCamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/OrthographicCameraController.cpp
)

target_link_libraries(Renderer
        PUBLIC
        RDE::Core
)