# RDE_Project/applications/sandbox/CMakeLists.txt

# The source file is now SandboxApp.cpp
add_executable(Sandbox SandboxApp.cpp)

target_sources(Sandbox PRIVATE
        src/systems/CameraSystem.cpp
        src/ui/ComponentUIRegistry.cpp
        src/EditorLayer.cpp
        src/SandboxLayer.cpp
)

target_link_libraries(Sandbox
        PRIVATE
        RDE::Core
        RDE::Renderer
        RDE::Platform::Glfw
        RDE::Platform::OpenGL
)