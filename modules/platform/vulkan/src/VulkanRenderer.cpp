#include "VulkanRenderer.h"
#include "AssetDatabase.h"

namespace RDE {

    VulkanRenderer::VulkanRenderer() {
        // The constructor's job is to create all the owned objects.
        m_device = std::make_unique<VulkanDevice>();
        m_texture_uploader = std::make_unique<VulkanTextureUploader>();
        // m_render_graph = std::make_unique<RenderGraph>(*m_device);
        // ... create command buffers, etc.
    }

    VulkanRenderer::~VulkanRenderer() {
        // unique_ptr will handle destruction in the correct order.
        // We might need to call device->wait_idle() here to be safe.
       shutdown();
    }

    void VulkanRenderer::prepare_resources(AssetDatabase& asset_db) {
        // This is the implementation of the abstract contract.
        // It calls its internal helper systems.
        m_texture_uploader->process_uploads(asset_db, *m_device);
    }

    void VulkanRenderer::draw_frame() {
        // This is where the "Hello, Triangle" logic from our plan will go.
        // 1. Acquire swapchain image
        RAL::TextureHandle backbuffer = m_device->acquire_next_swapchain_image();
        if (!backbuffer.is_valid()) {
            // Window was resized or something went wrong, skip frame.
            return;
        }

        // 2. Get a command buffer
        // auto cmd = get_command_buffer();

        // 3. Record drawing commands (proto-RenderGraph)
        // cmd->begin();
        // ... begin_render_pass with backbuffer as target ...
        // ... bind pipeline, bind vertex buffer, draw ...
        // ... end_render_pass ...
        // cmd->end();

        // 4. Submit command buffer
        // m_device->submit_command_buffers({ cmd });

        // 5. Present
        // m_device->present();
    }

    RAL::Device* VulkanRenderer::get_device() {
        return m_device.get();
    }

    void VulkanRenderer::shutdown() {
        // This is where we would clean up resources if needed.
        // The destructor will handle most of it, but we can do additional cleanup here.
        if (m_device) {
            m_device->wait_idle();
        }
        m_texture_uploader.reset();
        m_device.reset();
    }

} // namespace RDE