# RDE_Project/applications/sandbox/CMakeLists.txt

# The source file is now SandboxApp.cpp
add_executable(Sandbox SandboxApp.cpp)

target_link_libraries(Sandbox
        PRIVATE
        RDE::Core
        RDE::Renderer
        RDE::Platform::Glfw
        RDE::Platform::OpenGL
)