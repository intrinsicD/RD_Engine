#pragma once

#include "ral/Device.h"
#include "ral/CommandBuffer.h"
#include "VulkanTypes.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h> // Include the VMA header
#include <memory>


namespace RDE {
    // This will be our concrete implementation of the RAL::Device interface
    class VulkanDevice : public RAL::Device {
    public:
        VulkanDevice(); // Constructor will handle initialization

        ~VulkanDevice() override;

        // --- Swapchain Management (to be implemented) ---
        void create_swapchain(const RAL::SwapchainDescription &desc) override;

        void destroy_swapchain() override;

        RAL::TextureHandle acquire_next_swapchain_image() override;

        void present() override;

        // --- Resource Factories (to be implemented) ---
        RAL::BufferHandle create_buffer(const RAL::BufferDescription &desc) override;

        RAL::TextureHandle create_texture(const RAL::TextureDescription &desc) override;

        RAL::PipelineHandle create_pipeline(const RAL::PipelineDescription &desc) override;

        RAL::DescriptorSetLayoutHandle create_descriptor_set_layout(const RAL::DescriptorSetLayoutDescription &desc) override;

        void destroy_descriptor_set_layout(RAL::DescriptorSetLayoutHandle handle) override;

        std::vector<RAL::DescriptorSetHandle> allocate_descriptor_sets(
            RAL::DescriptorSetLayoutHandle layout, uint32_t count) override;

        void update_descriptor_sets(const std::vector<RAL::WriteDescriptorSet> &writes) override;
        // ...

        // --- Resource Destruction (to be implemented) ---
        void destroy_buffer(RAL::BufferHandle handle) override;

        void destroy_texture(RAL::TextureHandle handle) override;

        void destroy_pipeline(RAL::PipelineHandle handle) override;
        // ...

        // ... other pure virtual functions from RAL::Device ...
        std::unique_ptr<RAL::CommandBuffer> create_command_buffer() override;

        void submit_command_buffers(const std::vector<RAL::CommandBuffer *> &command_buffers) override;

        void wait_idle() override;

        // --- Vulkan-specific getters (for internal use by other Vulkan backend parts) ---
        VkDevice get_logical_device() const { return m_device; }

        VkPhysicalDevice get_physical_device() const { return m_physical_device; }

        VmaAllocator get_allocator() const { return m_allocator; }

    private:
        void pick_physical_device();

        void create_logical_device();

        void create_allocator();

        void cleanup_swapchain();

        void find_queue_families(VkPhysicalDevice device);

        VkInstance m_instance = VK_NULL_HANDLE;
        VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkQueue m_graphics_queue = VK_NULL_HANDLE;
        uint32_t m_graphics_queue_family_index = -1;

        VmaAllocator m_allocator = VK_NULL_HANDLE;

        // For debugging
        VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;

        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

        VkFormat m_swapchain_image_format;
        VkExtent2D m_swapchain_extent;

        // The swapchain provides us with a set of images to render into.
        // We need to store handles to them.
        std::vector<VkImage> m_swapchain_images;
        std::vector<VkImageView> m_swapchain_image_views;

        // We also need to map our abstract RAL::TextureHandles to these image views
        // so the RenderGraph can use them.
        std::vector<RAL::TextureHandle> m_swapchain_ral_textures;

        // --- Synchronization objects ---
        // We need semaphores to sync image acquisition and presentation, and a fence
        // to sync the CPU with the GPU rendering for a frame.
        static const int MAX_FRAMES_IN_FLIGHT = 2;
        std::vector<VkSemaphore> m_image_available_semaphores;
        std::vector<VkSemaphore> m_render_finished_semaphores;
        std::vector<VkFence> m_in_flight_fences;
        uint32_t m_current_frame = 0;
        uint32_t m_current_swapchain_image_index = 0;

        struct PImpl;
        std::unique_ptr<PImpl> m_pimpl;
    };
}