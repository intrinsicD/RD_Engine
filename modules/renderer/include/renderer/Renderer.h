//renderer/Renderer.h
#pragma once

#include "ral/Device.h"
#include "ral/CommandBuffer.h"
#include "core/IWindow.h"

namespace RDE {
    class Renderer {
    public:
        explicit Renderer(IWindow* window);

        ~Renderer() = default;

        void init();

        void shutdown();

        // High-level frame control
        RAL::CommandBuffer* begin_frame(); // Returns true if rendering is possible

        void end_frame(const std::vector<RAL::CommandBuffer *> &command_buffers);

        // The new high-level "draw call"
        // For now, let's make it simple. It will evolve later.
        void submit(RAL::PipelineHandle pipeline, RAL::BufferHandle vertexBuffer, RAL::BufferHandle indexBuffer,
                    uint32_t indexCount);

        // This allows access for systems that *truly* need the low-level device, like ImGui
        RAL::Device *get_device() { return m_device.get(); }

    private:
        IWindow *m_window = nullptr; // Pointer to the window, not owned by Renderer
        std::unique_ptr<RAL::Device> m_device;
        RAL::CommandBuffer *m_CurrentFrameCommandBuffer = nullptr;
    };
}