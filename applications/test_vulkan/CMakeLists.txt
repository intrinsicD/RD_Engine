# RDE_Project/applications/sandbox/CMakeLists.txt

# The source file is now SandboxApp.cpp
add_executable(TestVulkan)

target_sources(TestVulkan PRIVATE
    TestVulkanApp.cpp
)

target_link_libraries(TestVulkan
        PRIVATE
        RDE::Platform::Vulkan
        RDE::Renderer
        RDE::AssetSystem
        RDE::Core
        RDE::Platform::Glfw
        RDE::Engine
)