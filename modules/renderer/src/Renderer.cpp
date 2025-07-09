#include "renderer/Renderer.h"
#include "VulkanDevice.h"
#include "core/Log.h"

namespace RDE{
    Renderer::Renderer(IWindow* window) : m_window(window) {

    }

    void Renderer::init() {
        // Initialize the renderer, set up the device, create swapchain, and also all the imgui stuff
        RDE_CORE_INFO("Renderer::Init - Initializing Rendering Systems...");

        // 1. Create and own the Device
        auto window_handle = m_window->get_native_handle();
        m_Device = std::make_unique<VulkanDevice>(static_cast<GLFWwindow *>(window_handle));

        // 2. Create the Swapchain
        RAL::SwapchainDescription swapDesc{};
        swapDesc.nativeWindowHandle = window_handle;
        swapDesc.vsync = true; // Or get from config
        m_Device->create_swapchain(swapDesc);
    }

    void Renderer::shutdown() {
        if (!m_Device) return;

        RDE_CORE_INFO("Renderer::Shutdown - Shutting Down Rendering Systems...");
        m_Device->wait_idle();

        // The unique_ptr for m_Device will handle its own destruction
        // when the Renderer is destroyed.
        m_Device.reset();
    }

    RAL::CommandBuffer* Renderer::begin_frame() {
        m_CurrentFrameCommandBuffer = m_Device->begin_frame();
        return m_CurrentFrameCommandBuffer;
    }

    void Renderer::end_frame() {
        if (m_CurrentFrameCommandBuffer) {
            m_Device->end_frame();
        }
        m_CurrentFrameCommandBuffer = nullptr;
    }

    void Renderer::submit(RAL::PipelineHandle pipeline, RAL::BufferHandle vertexBuffer, RAL::BufferHandle indexBuffer,
                          uint32_t indexCount) {
        // Submit the draw call with the provided pipeline and buffers
        // This is where the actual rendering happens

    }
}