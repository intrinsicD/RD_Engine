# RDE_Project/modules/platform/opengl/CMakeLists.txt

add_library(OpenGLPlatform
        src/OpenGLShader.cpp
        src/OpenGLBuffer.cpp
        src/OpenGLRenderer.cpp
        src/OpenGLVertexArray.cpp
        src/OpenGLGraphicsAPI.cpp
        src/OpenGLTexture.cpp
        # We will add OpenGLBuffer.cpp and OpenGLVertexArray.cpp here later.
)
add_library(RDE::Platform::OpenGL ALIAS OpenGLPlatform)

# This module needs to know where the abstract Renderer interfaces are.
target_include_directories(OpenGLPlatform
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${PROJECT_SOURCE_DIR}/modules/renderer/include
        ${stb_SOURCE_DIR}
)

# This module implements the renderer, so it links against the graphics libraries.
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(OpenGLPlatform
        PUBLIC
        RDE::Renderer
        RDE::Engine
        Threads::Threads
        glad_gl_core
        OpenGL::GL
)