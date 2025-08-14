//vulkan/VulkanDevice.h
#pragma once

#include "ral/Device.h"
#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include "VulkanResourceManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDeletionQueue.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <functional>
#include <unordered_map>

struct GLFWwindow; // Forward declaration of GLFWwindow

namespace RDE {
    // This will be our concrete implementation of the RAL::Device interface
    class VulkanDevice final : public RAL::Device {
    public:
        // The constructor will handle all the initialization.
        explicit VulkanDevice(std::shared_ptr<VulkanContext> context, std::shared_ptr<VulkanSwapchain> swapchain);

        ~VulkanDevice() override;

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

        // Note: Descriptor sets are often allocated from a pool for efficiency.
        // This simple create function is a good starting point.
        RAL::DescriptorSetHandle create_descriptor_set(const RAL::DescriptorSetDescription &desc) override;

        void destroy_descriptor_set(RAL::DescriptorSetHandle handle) override;

        // --- Immediate Operations ---
        void immediate_submit(std::function<void(RAL::CommandBuffer *cmd)> &&function) override;

        void submit_and_wait(const std::vector<RAL::CommandBuffer*>& command_buffers) override;

        void *map_buffer(RAL::BufferHandle handle) override;

        void unmap_buffer(RAL::BufferHandle handle) override;

        void update_buffer_data(RAL::BufferHandle targetBuffer, const void *data, size_t size,
                                size_t offset = 0) override;

        VulkanSwapchain &get_swapchain() {
            return *m_Swapchain;
        }

    private:
        void submit_internal(const std::vector<VkCommandBuffer> &vkCommandBuffers);
        std::vector<RAL::TextureHandle> m_SwapchainTextureHandles;
        std::vector<RAL::TextureHandle> m_SwapchainDepthTextureHandles; // NEW per-swapchain-image depth textures
        void create_swapchain_texture_handles();
        void destroy_swapchain_texture_handles();
        void create_depth_textures(); // NEW
        void destroy_depth_textures(); // NEW

        //--------------------------------------------------------------------------------------------------------------
        // --- Core Dependencies (not owned) ---
        std::shared_ptr<VulkanContext> m_Context;
        std::shared_ptr<VulkanSwapchain> m_Swapchain;

        // --- Owned Vulkan Objects ---
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

        // --- Immediate Submit Context ---
        VkCommandPool m_UploadCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer m_UploadCommandBuffer = VK_NULL_HANDLE;
        VkFence m_UploadFence = VK_NULL_HANDLE;

        // --- Frame Sync & Management ---
        static constexpr int FRAMES_IN_FLIGHT = 2;
        uint32_t m_CurrentFrameIndex = 0;
        std::vector<std::unique_ptr<VulkanCommandBuffer> > m_FrameCommandBuffers;
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        // --- Deferred Deletion ---
        std::vector<DeletionQueue> m_FrameDeletionQueues;

        DeletionQueue &get_current_frame_deletion_queue();

        VkShaderModule create_shader_module(const std::vector<char> &code);

        // Descriptor set layout cache (hash -> handle)
        struct CachedLayoutEntry { RAL::DescriptorSetLayoutHandle handle; size_t refCount; RAL::DescriptorSetLayoutDescription desc; }; // NEW
        std::unordered_map<size_t, CachedLayoutEntry> m_DescriptorSetLayoutCache; // CHANGED
    };
}
