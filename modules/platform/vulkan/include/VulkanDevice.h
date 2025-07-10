//vulkan/VulkanDevice.h
#pragma once

#include "ral/Device.h"
#include "VulkanCommandBuffer.h"
#include "VulkanTypes.h"
#include "VulkanResourceManager.h"
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
        explicit VulkanDevice(GLFWwindow *window);

        ~VulkanDevice() override;

        void *map_buffer(RAL::BufferHandle handle) override;

        void unmap_buffer(RAL::BufferHandle handle) override;

        // We will implement these functions one by one. For now, we'll leave them as overrides.
        void create_swapchain(const RAL::SwapchainDescription &desc) override;

        void recreate_swapchain() override;

        void destroy_swapchain() override;

        RAL::TextureHandle acquire_next_swapchain_image() override;

        void present() override;

        std::unique_ptr<RAL::CommandBuffer> create_command_buffer() override;

        void submit(const std::vector<RAL::CommandBuffer*> &command_buffers) override;

        void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function); // Keep this declaration for now

        RAL::CommandBuffer *begin_frame() override;

        void end_frame() override;

        void wait_idle() override;

        // ... etc. for CreateBuffer, CreateTexture ...
        RAL::BufferHandle create_buffer(const RAL::BufferDescription &desc) override;

        void destroy_buffer(RAL::BufferHandle handle) override;

        RAL::TextureHandle create_texture(const RAL::TextureDescription &desc) override;

        void destroy_texture(RAL::TextureHandle handle) override;

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

        RAL::SamplerHandle create_sampler(const RAL::SamplerDescription &desc) override;

        void destroy_sampler(RAL::SamplerHandle handle) override;

    private:
        friend class VulkanCommandBuffer;

        static constexpr int FRAMES_IN_FLIGHT = 2; // For double buffering
        uint32_t m_CurrentFrameIndex{0}; // Current frame index for double buffering

        std::vector<DeletionQueue> m_FrameDeletionQueues;

        std::vector<std::unique_ptr<VulkanCommandBuffer>> m_FrameCommandBuffers;
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        // --- Core Vulkan Objects ---
        VkInstance m_Instance{VK_NULL_HANDLE};
        VkDebugUtilsMessengerEXT m_DebugMessenger{VK_NULL_HANDLE};
        VkSurfaceKHR m_Surface{VK_NULL_HANDLE};
        VkPhysicalDevice m_PhysicalDevice{VK_NULL_HANDLE};
        VkDevice m_LogicalDevice{VK_NULL_HANDLE};
        VkQueue m_GraphicsQueue{VK_NULL_HANDLE};
        VkQueue m_PresentQueue{VK_NULL_HANDLE};
        VmaAllocator m_VmaAllocator{VK_NULL_HANDLE};
        uint32_t m_GraphicsQueueFamilyIndex;

        VkPhysicalDeviceProperties m_PhysicalDeviceProperties;

        // We'll need these later for resource creation
        ResourceManager<VulkanBuffer, RAL::BufferHandle> m_BufferManager;
        ResourceManager<VulkanShader, RAL::ShaderHandle> m_ShaderManager;
        ResourceManager<VulkanPipeline, RAL::PipelineHandle> m_PipelineManager;
        ResourceManager<VulkanTexture, RAL::TextureHandle> m_TextureManager;
        ResourceManager<VulkanSampler, RAL::SamplerHandle> m_SamplerManager;
        ResourceManager<VulkanDescriptorSetLayout, RAL::DescriptorSetLayoutHandle> m_DsLayoutManager;
        ResourceManager<VkDescriptorSet, RAL::DescriptorSetHandle> m_DescriptorSetManager;

        VkDescriptorPool m_DescriptorPool{VK_NULL_HANDLE};
        GLFWwindow *m_Window{nullptr};

        // --- Command & Render Pass Infrastructure ---
        VkCommandPool m_CommandPool{VK_NULL_HANDLE};

        //Removed Swapchain Render Pass and Framebuffers because we will use Dynamic Rendering
       /* VkRenderPass m_SwapchainRenderPass{VK_NULL_HANDLE};
        std::vector<VkFramebuffer> m_SwapchainFramebuffers;*/

        VulkanSwapchain m_Swapchain;
        std::vector<RAL::TextureHandle> m_SwapchainTextureHandles;
        uint32_t m_CurrentImageIndex; // The Swapchain image index we get from AcquireNext, different from m_CurrentFrameIndex

        VkFence m_UploadFence{VK_NULL_HANDLE};
        VkCommandPool m_UploadCommandPool{VK_NULL_HANDLE};
        VkCommandBuffer m_UploadCommandBuffer{VK_NULL_HANDLE};

        // --- Private Helper Functions (we'll declare them here) ---
        DeletionQueue& get_current_frame_deletion_queue();

        void copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        VkShaderModule create_shader_module(const std::vector<char> &code);
    };
}
