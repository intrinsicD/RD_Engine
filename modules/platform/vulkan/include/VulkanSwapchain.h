#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct GLFWwindow; // Forward declaration of GLFWwindow
namespace RDE {
    class VulkanContext; // Forward-declare

    class VulkanSwapchain {
    public:
        // Depends on the context for device handles and the window for extent.
        VulkanSwapchain(VulkanContext *context, void *window, bool vsync = true);

        ~VulkanSwapchain();

        void recreate();

        // Returns false if swapchain is out of date
        VkResult acquire_next_image(VkSemaphore imageAvailableSemaphore, uint32_t *pImageIndex);

        VkResult present(VkSemaphore renderFinishedSemaphore, VkQueue presentQueue, uint32_t imageIndex);

        VkSwapchainKHR get_handle() const { return m_SwapchainHandle; }

        VkFormat get_image_format() const { return m_ImageFormat; }

        VkExtent2D get_extent() const { return m_Extent; }

        const std::vector<VkImage> &get_images() const { return m_Images; }

        const std::vector<VkImageView> &get_image_views() const { return m_ImageViews; }

    private:
        void create();

        void destroy();

        VulkanContext *m_Context; // Non-owning pointer
        GLFWwindow *m_Window; // Non-owning pointer

        bool m_VsyncEnabled = true;
        VkSwapchainKHR m_SwapchainHandle = VK_NULL_HANDLE;
        VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D m_Extent = {0, 0};

        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;
    };
}
