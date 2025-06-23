# RDE_Project/applications/sandbox/CMakeLists.txt

# The source file is now SandboxApp.cpp
add_executable(Sandbox SandboxApp.cpp)

target_sources(Sandbox PRIVATE
        src/systems/RenderSystem.cpp
        src/systems/CameraSystem.cpp
        src/ui/ComponentUIRegistry.cpp
        src/EditorLayer.cpp
        src/SandboxLayer.cpp
)

find_package(Threads REQUIRED)

target_link_libraries(Sandbox
        PRIVATE
        Threads::Threads
        RDE::Engine
        RDE::Renderer
        RDE::Platform::Glfw
        RDE::Platform::OpenGL
)