#pragma once

#include "Common.h"
#include "RenderPass.h"
#include "Barrier.h"

#include <string>
#include <cstdint>

namespace RAL {

    // An abstract interface for a command buffer.
    // The implementation (e.g., VulkanCommandBuffer, OpenGLCommandBuffer) will inherit from this.
    class CommandBuffer {
    public:
        virtual ~CommandBuffer() = default;

        // --- Core Lifecycle ---

        // Resets the command buffer to a recordable state. Must be called before recording begins.
        // In Vulkan, this resets the VkCommandBuffer. In OpenGL, it might be a no-op.
        virtual void begin() = 0;

        // Finalizes the command buffer for submission to a queue.
        virtual void end() = 0;

        // --- Render Pass Management ---

        // Begins a render pass instance. All subsequent draw calls will render into its attachments.
        // In Vulkan, this calls vkCmdBeginRenderPass.
        // In OpenGL, this binds a Framebuffer Object (FBO) and sets glClear values.
        virtual void begin_render_pass(const RenderPassBeginInfo& begin_info) = 0;

        // Ends the current render pass instance.
        virtual void end_render_pass() = 0;

        // --- State Binding ---

        // Binds a full Pipeline State Object. This is the most important state change command.
        // It sets shaders, vertex formats, rasterizer state, depth/stencil state, blend state, etc.
        virtual void bind_pipeline(PipelineHandle pipeline) = 0;

        // Binds a descriptor set to a specific set index (binding point).
        // A descriptor set contains pointers to resources like textures and buffers.
        // This maps to vkCmdBindDescriptorSets. OpenGL would have to emulate this.
        virtual void bind_descriptor_set(uint32_t set_index, DescriptorSetHandle descriptor_set) = 0;

        // Binds a vertex buffer to a specific binding index.
        virtual void bind_vertex_buffer(uint32_t binding_index, BufferHandle buffer, uint64_t offset = 0) = 0;

        // Binds an index buffer for indexed drawing.
        virtual void bind_index_buffer(BufferHandle buffer, IndexType index_type, uint64_t offset = 0) = 0;

        // --- Drawing Commands ---

        // The fundamental non-indexed draw call.
        virtual void draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) = 0;

        // The fundamental indexed draw call.
        virtual void draw_indexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0, uint32_t first_instance = 0) = 0;

        // --- Viewport & Scissor ---

        // Sets the viewport dynamically.
        virtual void set_viewport(const Viewport& viewport) = 0;

        // Sets the scissor rectangle dynamically.
        virtual void set_scissor(const Rect2D& scissor) = 0;

        // --- Synchronization (Advanced) ---

        // Inserts a pipeline barrier. This is the key to synchronization.
        // The RenderGraph would call this automatically between passes.
        virtual void pipeline_barrier(const BarrierInfo& barrier_info) = 0;

        // --- Debugging ---
        // These map to VK_EXT_debug_utils and KHR_debug for other APIs. Invaluable for tools like RenderDoc.

        virtual void begin_debug_label(const std::string& label_name) = 0;
        virtual void end_debug_label() = 0;
        virtual void insert_debug_label(const std::string& label_name) = 0;
    };

} // namespace RAL