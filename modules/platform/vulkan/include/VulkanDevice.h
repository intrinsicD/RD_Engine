//vulkan/VulkanDevice.h
#pragma once

#include "ral/Device.h"
#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include "VulkanResourceManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanTypes.h"
#include "VulkanDeletionQueue.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h> // Include the VMA header
#include <memory>
#include <functional>

struct GLFWwindow; // Forward declaration of GLFWwindow

namespace RDE {
    class VulkanHelper;

    // This will be our concrete implementation of the RAL::Device interface
    class VulkanDevice final : public RAL::Device {
    public:
        // The constructor will handle all the initialization.
        explicit VulkanDevice(std::shared_ptr<VulkanContext> context, std::shared_ptr<VulkanSwapchain> swapchain);

        ~VulkanDevice() override;

        // --- Frame Lifecycle ---
        RAL::CommandBuffer *begin_frame() override;

        void end_frame(const std::vector<RAL::CommandBuffer *> &command_buffers) override;

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

        // --- Resource Access ---
        // Used by the high-level renderer to resolve handles before passing to a command buffer

        VulkanBuffer &get_buffer(RAL::BufferHandle handle) { return m_BufferManager.get(handle); }

        VulkanTexture &get_texture(RAL::TextureHandle handle) { return m_TextureManager.get(handle); }

        VulkanSampler &get_sampler(RAL::SamplerHandle handle) { return m_SamplerManager.get(handle); }

        VulkanShader &get_shader(RAL::ShaderHandle handle) { return m_ShaderManager.get(handle); }

        VulkanPipeline &get_pipeline(RAL::PipelineHandle handle) { return m_PipelineManager.get(handle); }

        VulkanDescriptorSetLayout &get_descriptor_set_layout(RAL::DescriptorSetLayoutHandle handle) {
            return m_DsLayoutManager.get(handle);
        }

        VkDescriptorSet get_descriptor_set(RAL::DescriptorSetHandle handle) {
            return m_DescriptorSetManager.get(handle);
        }

        VulkanSwapchain &get_swapchain() { return *m_Swapchain; }

        bool is_valid(RAL::BufferHandle handle) const {
            return m_BufferManager.is_valid(handle);
        }

        bool is_valid(RAL::TextureHandle handle) const {
            return m_TextureManager.is_valid(handle);
        }

        bool is_valid(RAL::SamplerHandle handle) const {
            return m_SamplerManager.is_valid(handle);
        }

        bool is_valid(RAL::ShaderHandle handle) const {
            return m_ShaderManager.is_valid(handle);
        }

        bool is_valid(RAL::PipelineHandle handle) const {
            return m_PipelineManager.is_valid(handle);
        }

        bool is_valid(RAL::DescriptorSetLayoutHandle handle) const {
            return m_DsLayoutManager.is_valid(handle);
        }

        bool is_valid(RAL::DescriptorSetHandle handle) const {
            return m_DescriptorSetManager.is_valid(handle);
        }

        // ... other getters for raw Vulkan handles ...

        // --- Immediate Operations ---
        void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

        void *map_buffer(RAL::BufferHandle handle) override;

        void unmap_buffer(RAL::BufferHandle handle) override;

    private:
        void submit_internal(const std::vector<VkCommandBuffer> &vkCommandBuffers);

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
        std::vector<std::unique_ptr<VulkanCommandBuffer>> m_FrameCommandBuffers;
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        // --- Resource Management ---
        ResourceManager<VulkanBuffer, RAL::BufferHandle> m_BufferManager;
        ResourceManager<VulkanShader, RAL::ShaderHandle> m_ShaderManager;
        ResourceManager<VulkanPipeline, RAL::PipelineHandle> m_PipelineManager;
        ResourceManager<VulkanTexture, RAL::TextureHandle> m_TextureManager;
        ResourceManager<VulkanSampler, RAL::SamplerHandle> m_SamplerManager;
        ResourceManager<VulkanDescriptorSetLayout, RAL::DescriptorSetLayoutHandle> m_DsLayoutManager;
        ResourceManager<VkDescriptorSet, RAL::DescriptorSetHandle> m_DescriptorSetManager;

        // --- Deferred Deletion ---
        std::vector<DeletionQueue> m_FrameDeletionQueues;

        DeletionQueue &get_current_frame_deletion_queue();

        void copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

        VkShaderModule create_shader_module(const std::vector<char> &code);
    };
}
