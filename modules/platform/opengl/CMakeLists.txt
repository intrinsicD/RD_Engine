# RDE_Project/modules/platform/opengl/CMakeLists.txt

# This library contains our concrete OpenGL renderer implementation.
add_library(OpenGLPlatform)
add_library(RDE::Platform::OpenGL ALIAS OpenGLPlatform)

# This module needs to know where the abstract Renderer interfaces are.
target_include_directories(OpenGLPlatform
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${PROJECT_SOURCE_DIR}/modules/renderer/include
        ${PROJECT_SOURCE_DIR}/modules/engine/include
        ${stb_SOURCE_DIR}
)

target_sources(OpenGLPlatform PRIVATE
        src/OpenGLDevice.cpp
        src/OpenGLCommandBuffer.cpp
)

# This module implements the renderer, so it links against the graphics libraries.
find_package(OpenGL REQUIRED)
target_link_libraries(OpenGLPlatform
        PUBLIC
        OpenGL::GL
        RDE::Renderer
        RDE::Core
        glad_gl_core
        glfw
)