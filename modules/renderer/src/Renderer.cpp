#include "renderer/Renderer.h"
#include "renderer/RendererComponentTypes.h"
#include "VulkanDevice.h"
#include "core/Log.h"
#include "renderer/ShaderData.h"

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

        // Camera resources
        init_camera_resources();
    }

    void Renderer::shutdown() {
        if (!m_device) return;

        // destroy camera resources before device reset
        destroy_camera_resources();

        RDE_CORE_INFO("Renderer::Shutdown - Shutting Down Rendering Systems...");
        m_device->wait_idle();

        // The unique_ptr for m_device will handle its own destruction
        // when the Renderer is destroyed.
        m_device.reset();
    }

    RAL::CommandBuffer *Renderer::begin_frame() {
        m_CurrentFrameContext = m_device->begin_frame();
        if (!m_CurrentFrameContext.swapchainTexture.is_valid()) {
            m_CurrentFrameCommandBuffer = nullptr;
            return nullptr; // Frame should be skipped
        }
        m_CurrentFrameCommandBuffer = m_device->get_command_buffer();
        return m_CurrentFrameCommandBuffer;
    }

    void Renderer::end_frame(const std::vector<RAL::CommandBuffer *> &command_buffers) {
        if (m_CurrentFrameCommandBuffer) {
            m_device->end_frame(m_CurrentFrameContext, command_buffers);
        }
        m_CurrentFrameCommandBuffer = nullptr;
    }

    void Renderer::update_camera(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec3 &camPos) {
        if(!m_cameraBuffer.is_valid()) return;
        CameraData data{};
        data.view = view;
        data.proj = proj;
        data.camPos = camPos;
        m_device->update_buffer_data(m_cameraBuffer, &data, sizeof(CameraData), 0);
    }

    void Renderer::init_camera_resources(){
        if(m_cameraBuffer.is_valid()) return; // already
        m_cameraBufferSize = sizeof(CameraData);
        RAL::BufferDescription bd{}; bd.size = m_cameraBufferSize; bd.usage = RAL::BufferUsage::UniformBuffer; bd.memoryUsage = RAL::MemoryUsage::HostVisibleCoherent;
        m_cameraBuffer = m_device->create_buffer(bd);
        // Create descriptor set layout (set=0, binding 0 uniform buffer vertex+fragment)
        RAL::DescriptorSetLayoutDescription layoutDesc{}; layoutDesc.set = 0;
        RAL::DescriptorSetLayoutBinding binding{}; binding.binding = 0; binding.type = RAL::DescriptorType::UniformBuffer; binding.stages = RAL::ShaderStage::Vertex | RAL::ShaderStage::Fragment; binding.name = "CameraData";
        layoutDesc.bindings.push_back(binding);
        m_cameraSetLayout = m_device->create_descriptor_set_layout(layoutDesc);
        // Allocate descriptor set
        RAL::DescriptorSetDescription setDesc{}; setDesc.layout = m_cameraSetLayout;
        RAL::DescriptorWrite write{}; write.binding = 0; write.type = RAL::DescriptorType::UniformBuffer; write.buffer = m_cameraBuffer;
        setDesc.writes.push_back(write);
        m_cameraDescriptorSet = m_device->create_descriptor_set(setDesc);
    }

    void Renderer::destroy_camera_resources(){
        if(!m_device) return;
        if(m_cameraDescriptorSet.is_valid()) m_device->destroy_descriptor_set(m_cameraDescriptorSet);
        if(m_cameraSetLayout.is_valid()) m_device->destroy_descriptor_set_layout(m_cameraSetLayout);
        if(m_cameraBuffer.is_valid()) m_device->destroy_buffer(m_cameraBuffer);
        m_cameraDescriptorSet = RAL::DescriptorSetHandle::INVALID();
        m_cameraSetLayout = RAL::DescriptorSetLayoutHandle::INVALID();
        m_cameraBuffer = RAL::BufferHandle::INVALID();
        m_cameraBufferSize = 0;
    }

    void Renderer::render(const View &view) {
        if (!m_CurrentFrameCommandBuffer) return;

        bool haveDepth = m_CurrentFrameContext.depthTexture.is_valid();
        RAL::Format requiredDepthFormat = haveDepth ? RAL::Format::D32_SFLOAT : RAL::Format::UNKNOWN;
        auto &db = m_device->get_resources_database();

        // State caching
        const RenderGpuMaterial *last_material = nullptr;

        for (const auto &packet: view) {
            // --- Ensure pipeline depth format matches current render pass usage ---
            if(haveDepth && packet.material) {
                if(db.is_valid(packet.material->pipeline_id)) {
                    auto &pipDesc = db.get<RAL::PipelineDescription>(packet.material->pipeline_id);
                    if(pipDesc.depthAttachmentFormat == RAL::Format::UNKNOWN) {
                        // Rebuild pipeline with depth format injected.
                        RAL::PipelineDescription newDesc = pipDesc; // copy
                        newDesc.depthAttachmentFormat = requiredDepthFormat;
                        RAL::PipelineHandle newPipeline = m_device->create_pipeline(newDesc);
                        if(newPipeline.is_valid()) {
                            // Schedule old pipeline for destruction
                            m_device->destroy_pipeline(packet.material->pipeline_id);
                            // Replace in material (const_cast because packet.material is const ptr view)
                            auto *mutableMat = const_cast<RenderGpuMaterial*>(packet.material);
                            mutableMat->pipeline_id = newPipeline;
                            // Update cached description for new pipeline
                            db.emplace_or_replace<RAL::PipelineDescription>(newPipeline, newDesc);
                            last_material = nullptr; // force rebind
                        }
                    }
                }
            }

            if (packet.material != last_material) {
                m_CurrentFrameCommandBuffer->bind_pipeline(packet.material->pipeline_id);
                if(m_cameraDescriptorSet.is_valid()) {
                    m_CurrentFrameCommandBuffer->bind_descriptor_set(packet.material->pipeline_id, m_cameraDescriptorSet, 0);
                }
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