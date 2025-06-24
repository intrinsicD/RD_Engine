# RDE_Project/applications/sandbox/CMakeLists.txt

# The source file is now SandboxApp.cpp
add_executable(Sandbox)

target_sources(Sandbox PRIVATE
        SandboxApp.cpp
        src/ui/ComponentUIRegistry.cpp
        src/EditorLayer.cpp
        src/SandboxLayer.cpp
)

target_link_libraries(Sandbox
        PRIVATE
        RDE::Engine
        RDE::Renderer
        RDE::Platform::Glfw
        RDE::Platform::OpenGL
)