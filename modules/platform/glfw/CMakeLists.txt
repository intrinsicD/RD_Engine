# RDE_Project/modules/platform/glfw/CMakeLists.txt

# This library contains our concrete GLFW window implementation.
add_library(GlfwPlatform)
add_library(RDE::Platform::Glfw ALIAS GlfwPlatform)

# It needs access to the public headers of our Core library (for the Window interface).
target_include_directories(GlfwPlatform
        PUBLIC
        # No public headers of its own to expose.
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${glm_SOURCE_DIR}
        PRIVATE
        # Needs to know where the Core library's headers are.
        ${PROJECT_SOURCE_DIR}/modules/core/include
)

# This is a platform implementation, so it links against the actual platform libraries.
find_package(OpenGL REQUIRED)

target_sources(GlfwPlatform PRIVATE
        GlfwWindow.cpp
        GlfwInput.cpp
)

target_link_libraries(GlfwPlatform
        PUBLIC
        # It needs to link to the same dependencies as Core did for windowing.
        RDE::Core
        glfw
        glad_gl_core
        OpenGL::GL
)