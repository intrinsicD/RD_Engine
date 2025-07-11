# RDE_Project/modules/platform/opengl/CMakeLists.txt

# This library contains our concrete OpenGL renderer implementation.
add_library(VulkanPlatform)
add_library(RDE::Platform::Vulkan ALIAS VulkanPlatform)

# This module needs to know where the abstract Renderer interfaces are.
target_include_directories(VulkanPlatform
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${PROJECT_SOURCE_DIR}/modules/renderer/include
       ${VMA_SOURCE_DIR}/include
)

target_sources(VulkanPlatform PRIVATE
        src/VulkanCommandBuffer.cpp
        src/VulkanDevice.cpp
        src/VulkanDeletionQueue.cpp
        src/VulkanSwapchain.cpp
        src/VulkanContext.cpp
)

# This module implements the renderer, so it links against the graphics libraries.
find_package(Vulkan REQUIRED)
target_link_libraries(VulkanPlatform
        PUBLIC
        Vulkan::Vulkan
        RDE::Renderer
        RDE::Core
        glfw
)