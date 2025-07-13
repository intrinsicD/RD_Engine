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
        auto window_handle = static_cast<GLFWwindow *>(m_window->get_native_handle());
        auto context = std::make_shared<VulkanContext>(window_handle);
        auto swapchain = std::make_shared<VulkanSwapchain>(context.get(), window_handle);
        m_device = std::make_unique<VulkanDevice>(context, swapchain);
    }

    void Renderer::shutdown() {
        if (!m_device) return;

        RDE_CORE_INFO("Renderer::Shutdown - Shutting Down Rendering Systems...");
        m_device->wait_idle();

        // The unique_ptr for m_device will handle its own destruction
        // when the Renderer is destroyed.
        m_device.reset();
    }

    RAL::CommandBuffer* Renderer::begin_frame() {
        m_CurrentFrameCommandBuffer = m_device->begin_frame();
        return m_CurrentFrameCommandBuffer;
    }

    void Renderer::end_frame(const std::vector<RAL::CommandBuffer *> &command_buffers) {
        if (m_CurrentFrameCommandBuffer) {
            m_device->end_frame(command_buffers);
        }
        m_CurrentFrameCommandBuffer = nullptr;
    }

    void Renderer::submit(RAL::PipelineHandle pipeline, RAL::BufferHandle vertexBuffer, RAL::BufferHandle indexBuffer,
                          uint32_t indexCount) {
        // Submit the draw call with the provided pipeline and buffers
        // This is where the actual rendering happens

    }
}