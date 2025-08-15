//renderer/Renderer.h
#pragma once

#include "ral/Device.h"
#include "ral/CommandBuffer.h"
#include "core/IWindow.h"
#include "RenderPacket.h"

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

        void render(const View &view);
        void update_camera(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec3 &camPos);

        // Access camera descriptor resources
        RAL::DescriptorSetLayoutHandle get_camera_set_layout() const { return m_cameraSetLayout; }
        RAL::DescriptorSetHandle get_camera_descriptor_set() const { return m_cameraDescriptorSet; }

        // This allows access for systems that *truly* need the low-level device, like ImGui
        RAL::Device *get_device() { return m_device.get(); }

        const RAL::FrameContext& get_current_frame_context() const { return m_CurrentFrameContext; }


    private:
        IWindow *m_window = nullptr; // Pointer to the window, not owned by Renderer
        std::unique_ptr<RAL::Device> m_device;
        RAL::CommandBuffer *m_CurrentFrameCommandBuffer = nullptr;
        RAL::FrameContext m_CurrentFrameContext;

        // Camera UBO resources (set = 0, binding = 0)
        RAL::BufferHandle m_cameraBuffer{RAL::BufferHandle::INVALID()};
        RAL::DescriptorSetLayoutHandle m_cameraSetLayout{RAL::DescriptorSetLayoutHandle::INVALID()};
        RAL::DescriptorSetHandle m_cameraDescriptorSet{RAL::DescriptorSetHandle::INVALID()};
        size_t m_cameraBufferSize = 0;
        void init_camera_resources();
        void destroy_camera_resources();
        void init_camera_resources_from_layout(RAL::DescriptorSetLayoutHandle layout);
    };
}