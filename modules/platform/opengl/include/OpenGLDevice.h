//opengl/OpenGLDevice.h
#pragma once

#include "ral/Device.h"

namespace RDE{
    class OpenGLDevice final : public RAL::Device {
    public:
        OpenGLDevice();

        ~OpenGLDevice() override = default;

        // --- Frame Lifecycle ---
        RAL::FrameContext begin_frame() override;

        void end_frame(const RAL::FrameContext &context,
                       const std::vector<RAL::CommandBuffer *> &command_buffers) override;

        RAL::CommandBuffer *get_command_buffer() override;

        void wait_idle() override;

        // --- Swapchain Management ---
        void recreate_swapchain() override;

        // --- Resource Creation (implements RAL) ---
        RAL::BufferHandle create_buffer(const RAL::BufferDescription &desc) override;

        void destroy_buffer(RAL::BufferHandle handle) override;

        RAL::TextureHandle create_texture(const RAL::TextureDescription &desc) override;

        void destroy_texture(RAL::TextureHandle handle) override;

        RAL::SamplerHandle create_sampler(const RAL::SamplerDescription &desc) override;

        void destroy_sampler(RAL::SamplerHandle handle) override;

        RAL::ShaderHandle create_shader(const RAL::ShaderDescription &desc) override;

        RAL::ShaderHandle create_shader_module(const std::vector<char> &bytecode, RAL::ShaderStage stage) override;

        void destroy_shader(RAL::ShaderHandle handle) override;

        RAL::PipelineHandle create_pipeline(const RAL::PipelineDescription &desc) override;

        void destroy_pipeline(RAL::PipelineHandle handle) override;

        RAL::DescriptorSetLayoutHandle
        create_descriptor_set_layout(const RAL::DescriptorSetLayoutDescription &desc) override;

        void destroy_descriptor_set_layout(RAL::DescriptorSetLayoutHandle handle) override;

        RAL::DescriptorSetHandle create_descriptor_set(const RAL::DescriptorSetDescription &desc) override;

        void destroy_descriptor_set(RAL::DescriptorSetHandle handle) override;

        // --- Immediate Operations ---
        void immediate_submit(std::function<void(RAL::CommandBuffer *cmd)> &&function) override;

        void submit_and_wait(const std::vector<RAL::CommandBuffer*>& command_buffers) override;

        void *map_buffer(RAL::BufferHandle handle) override;

        void unmap_buffer(RAL::BufferHandle handle) override;

        void update_buffer_data(RAL::BufferHandle targetBuffer, const void *data, size_t size,
                                size_t offset = 0) override;
    };
}