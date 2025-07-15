#include "renderer/Renderer.h"
#include "renderer/RendererComponentTypes.h"
#include "VulkanDevice.h"
#include "core/Log.h"

namespace RDE {
    Renderer::Renderer(IWindow *window) : m_window(window) {

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

    RAL::CommandBuffer *Renderer::begin_frame() {
        m_CurrentFrameCommandBuffer = m_device->begin_frame();
        return m_CurrentFrameCommandBuffer;
    }

    void Renderer::end_frame(const std::vector<RAL::CommandBuffer *> &command_buffers) {
        if (m_CurrentFrameCommandBuffer) {
            m_device->end_frame(command_buffers);
        }
        m_CurrentFrameCommandBuffer = nullptr;
    }

    void Renderer::render(const View &view) {
        if (!m_CurrentFrameCommandBuffer) return;

        // State caching
        const RenderGpuMaterial *last_material = nullptr;

        for (const auto &packet: view) {
            // --- Bind Pipeline and Material Descriptor Set ---
            // This check also implicitly handles pipeline changes, since a material is tied to a pipeline.
            if (packet.material != last_material) {
                m_CurrentFrameCommandBuffer->bind_pipeline(packet.material->pipeline_id);

                // Assuming material data is in set 1
                m_CurrentFrameCommandBuffer->bind_descriptor_set(
                        packet.material->pipeline_id,
                        packet.material->descriptor_set_id,
                        1
                );
                last_material = packet.material;
            }

            // --- Push Constants for the model matrix ---
            m_CurrentFrameCommandBuffer->push_constants(
                    packet.material->pipeline_id,
                    RAL::ShaderStage::Vertex, 0, sizeof(glm::mat4), &packet.model_matrix
            );

            // --- BIND THE MULTIPLE VERTEX BUFFERS ---
            for (const auto &[attribute_id, buffer_handle]: packet.geometry->attribute_buffers) {
                // Look up the binding point for this attribute in the current material's layout map
                auto it = packet.material->attribute_to_binding_map.find(attribute_id);
                if (it != packet.material->attribute_to_binding_map.end()) {
                    uint32_t binding_point = it->second;
                    m_CurrentFrameCommandBuffer->bind_vertex_buffer(buffer_handle, binding_point);
                }
            }

            // --- Bind Index Buffer and Draw ---
            if (packet.geometry->index_buffer.is_valid()) {
                m_CurrentFrameCommandBuffer->bind_index_buffer(
                        packet.geometry->index_buffer,
                        packet.geometry->index_type
                );
                m_CurrentFrameCommandBuffer->draw_indexed(
                        packet.geometry->index_count, 1, 0, 0, 0
                );
            } else {
                m_CurrentFrameCommandBuffer->draw(
                        packet.geometry->vertex_count, 1, 0, 0
                );
            }
        }
    }
}