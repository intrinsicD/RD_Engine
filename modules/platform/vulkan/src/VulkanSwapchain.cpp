#include "VulkanSwapchain.h"
#include "VulkanContext.h"
#include "VulkanCommon.h"

#include <GLFW/glfw3.h>
#include <limits>
#include <algorithm>

namespace RDE {
    // Anonymous namespace for helper functions local to this translation unit
    namespace {
        struct SwapchainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
            SwapchainSupportDetails details;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                                          details.presentModes.data());
            }
            return details;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
            for (const auto &availableFormat: availableFormats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                    availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }
            }
            return availableFormats[0];
        }

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes, bool vsync) {
            if (!vsync) {
                for (const auto &availablePresentMode: availablePresentModes) {
                    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                        return availablePresentMode;
                    }
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            } else {
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);

                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                                capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                                 capabilities.maxImageExtent.height);

                return actualExtent;
            }
        }
    } // end anonymous namespace


    VulkanSwapchain::VulkanSwapchain(VulkanContext *context, void *window, bool vsync)
        : m_Context(context), m_Window(static_cast<GLFWwindow *>(window)), m_VsyncEnabled(vsync) {
        create();
    }

    VulkanSwapchain::~VulkanSwapchain() {
        destroy();
    }

    void VulkanSwapchain::create() {
        // MOVED FROM: VulkanDevice::create_swapchain
        VkSwapchainKHR oldSwapchain = m_SwapchainHandle;

        SwapchainSupportDetails support = querySwapchainSupport(m_Context->get_physical_device(),
                                                                m_Context->get_surface());
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(support.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(support.presentModes, m_VsyncEnabled);
        VkExtent2D extent = chooseSwapExtent(support.capabilities, m_Window);

        // If the window is minimized, the extent will be 0. We cannot create a swapchain then.
        // We will just wait until the window is restored and recreate() is called.
        if (extent.width == 0 || extent.height == 0) {
            return;
        }

        uint32_t imageCount = support.capabilities.minImageCount + 1;
        if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount) {
            imageCount = support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Context->get_surface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = support.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapchain; // Handles seamless recreation

        auto logicalDevice = m_Context->get_logical_device();
        VK_CHECK(vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &m_SwapchainHandle));

        // If we had an old swapchain, destroy it now.
        if (oldSwapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);
        }

        m_ImageFormat = surfaceFormat.format;
        m_Extent = extent;

        vkGetSwapchainImagesKHR(logicalDevice, m_SwapchainHandle, &imageCount, nullptr);
        m_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(logicalDevice, m_SwapchainHandle, &imageCount, m_Images.data());

        m_ImageViews.resize(m_Images.size());
        for (size_t i = 0; i < m_Images.size(); i++) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_Images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_ImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            VK_CHECK(vkCreateImageView(logicalDevice, &viewInfo, nullptr, &m_ImageViews[i]));
        }
    }

    void VulkanSwapchain::destroy() {
        // MOVED FROM: VulkanDevice::destroy_swapchain
        auto logicalDevice = m_Context->get_logical_device();

        // Ensure that resources are not in use before destroying them.
        vkDeviceWaitIdle(logicalDevice);

        for (auto imageView: m_ImageViews) {
            vkDestroyImageView(logicalDevice, imageView, nullptr);
        }
        m_ImageViews.clear();
        m_Images.clear(); // Handles are owned by the swapchain, no need to destroy

        if (m_SwapchainHandle != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(logicalDevice, m_SwapchainHandle, nullptr);
            m_SwapchainHandle = VK_NULL_HANDLE;
        }
    }

    void VulkanSwapchain::recreate() {
        // MOVED FROM: VulkanDevice::recreate_swapchain
        auto logicalDevice = m_Context->get_logical_device();
        vkDeviceWaitIdle(logicalDevice);

        // Clean up old image views before creating new ones
        for (auto imageView: m_ImageViews) {
            vkDestroyImageView(logicalDevice, imageView, nullptr);
        }
        m_ImageViews.clear();

        create(); // `create` handles passing the old swapchain handle for seamless transition
    }

    VkResult VulkanSwapchain::acquire_next_image(VkSemaphore imageAvailableSemaphore, uint32_t *pImageIndex) {
        auto logicalDevice = m_Context->get_logical_device();
        if (m_SwapchainHandle == VK_NULL_HANDLE) {
            return VK_ERROR_OUT_OF_DATE_KHR;
        }

        // Use the output parameter
        return vkAcquireNextImageKHR(logicalDevice, m_SwapchainHandle, UINT64_MAX, imageAvailableSemaphore,
                                     VK_NULL_HANDLE, pImageIndex);
    }

    VkResult VulkanSwapchain::present(VkSemaphore renderFinishedSemaphore, VkQueue presentQueue, uint32_t imageIndex){
        // MOVED FROM: VulkanDevice::present
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_SwapchainHandle;
        presentInfo.pImageIndices = &imageIndex;

        return vkQueuePresentKHR(presentQueue, &presentInfo);
    }
}
