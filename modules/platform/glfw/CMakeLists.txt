# RDE_Project/modules/platform/glfw/CMakeLists.txt

# This library contains our concrete GLFW window implementation.
add_library(GlfwPlatform)
add_library(RDE::Platform::Glfw ALIAS GlfwPlatform)

# It needs access to the public headers of our Core library (for the Window interface).
target_include_directories(GlfwPlatform
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${glm_SOURCE_DIR}
        PRIVATE
        # Needs to know where the Core library's headers are.
        ${PROJECT_SOURCE_DIR}/modules/engine/include
)

target_sources(GlfwPlatform PRIVATE
        src/GlfwVulkanWindow.cpp
        src/GlfwInput.cpp
)

# This is a platform implementation, so it links against the actual platform libraries.
find_package(OpenGL REQUIRED)
target_link_libraries(GlfwPlatform
        PUBLIC
        # It needs to link to the same dependencies as Core did for windowing.
        RDE::Engine
        glfw
        glad_gl_core
        OpenGL::GL
)