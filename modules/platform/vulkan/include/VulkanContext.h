//vulkan/VulkanContext.h

#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

namespace RDE {
    class VulkanContext {
    public:
        VulkanContext(GLFWwindow *window);

        ~VulkanContext();

        // --- Getters for core handles ---
        VkInstance get_instance() const { return m_Instance; }

        VkPhysicalDevice get_physical_device() const { return m_PhysicalDevice; }

        VkDevice get_logical_device() const { return m_LogicalDevice; }

        VmaAllocator get_vma_allocator() const { return m_VmaAllocator; }

        VkSurfaceKHR get_surface() const { return m_Surface; }

        uint32_t get_graphics_queue_family() const { return m_GraphicsQueueFamilyIndex; }

        VkQueue get_graphics_queue() const { return m_GraphicsQueue; }

        VkQueue get_present_queue() const { return m_PresentQueue; } // Add this getter

        VkPhysicalDeviceProperties get_physical_device_properties() const {
            return m_PhysicalDeviceProperties;
        }
    private:
        // All the logic from steps 1-5 of your original constructor goes here
        void create_instance();

        void setup_debug_messenger();

        void create_surface(GLFWwindow *window);

        void pick_physical_device();

        void create_logical_device();

        void create_vma_allocator();

        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkDevice m_LogicalDevice = VK_NULL_HANDLE;
        VmaAllocator m_VmaAllocator = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties m_PhysicalDeviceProperties{};

        uint32_t m_GraphicsQueueFamilyIndex;
        uint32_t m_PresentQueueFamilyIndex;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE; // Could be different
    };
}