# RDE_Project/modules/platform/opengl/CMakeLists.txt

# This library contains our concrete OpenGL renderer implementation.
add_library(OpenGLPlatform)
add_library(RDE::Platform::OpenGL ALIAS OpenGLPlatform)

# This module needs to know where the abstract Renderer interfaces are.
target_include_directories(OpenGLPlatform
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        PRIVATE
        ${PROJECT_SOURCE_DIR}/modules/renderer/include
        ${PROJECT_SOURCE_DIR}/modules/engine/include
        ${stb_SOURCE_DIR}
)

target_sources(OpenGLPlatform PRIVATE
        src/OpenGLShader.cpp
        src/OpenGLBuffer.cpp
        src/OpenGLRenderer.cpp
        src/OpenGLVertexArray.cpp
        src/OpenGLGraphicsAPI.cpp
        src/OpenGLTexture.cpp
)

# This module implements the renderer, so it links against the graphics libraries.
find_package(OpenGL REQUIRED)
target_link_libraries(OpenGLPlatform
        PUBLIC
        RDE::Engine
        RDE::Renderer
        glad_gl_core
        OpenGL::GL
)