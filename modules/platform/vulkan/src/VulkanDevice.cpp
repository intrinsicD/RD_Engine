#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommon.h"

#include "core/Log.h"
#include "core/FileIOUtils.h"
#include "ral/Resources.h"

// Place the VMA implementation macro here
#define VMA_IMPLEMENTATION

#include <vk_mem_alloc.h>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <memory>
#include <GLFW/glfw3.h>


namespace RDE {
    // A debug callback function for the validation layers
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData) {

        // Only print warnings and errors
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            RDE_CORE_ERROR("Validation Layer: {}", pCallbackData->pMessage);
        }
        return VK_FALSE;
    }

    VkFormat ToVulkanFormat(RAL::Format format) {
        // This would be a large switch statement.
        // Let's add a few examples.
        switch (format) {
            // 8-bit
            case RAL::Format::R8_UNORM:             return VK_FORMAT_R8_UNORM;
            case RAL::Format::R8G8_UNORM:           return VK_FORMAT_R8G8_UNORM;
            case RAL::Format::R8G8B8A8_UNORM:       return VK_FORMAT_R8G8B8A8_UNORM;
            case RAL::Format::B8G8R8A8_UNORM:       return VK_FORMAT_B8G8R8A8_UNORM;
            case RAL::Format::R8_SRGB:              return VK_FORMAT_R8_SRGB;
            case RAL::Format::R8G8_SRGB:            return VK_FORMAT_R8G8_SRGB;
            case RAL::Format::R8G8B8A8_SRGB:        return VK_FORMAT_R8G8B8A8_SRGB;
            case RAL::Format::B8G8R8A8_SRGB:        return VK_FORMAT_B8G8R8A8_SRGB;
                // 16-bit
            case RAL::Format::R16_SFLOAT:           return VK_FORMAT_R16_SFLOAT;
            case RAL::Format::R16G16_SFLOAT:        return VK_FORMAT_R16G16_SFLOAT;
            case RAL::Format::R16G16B16A16_SFLOAT:  return VK_FORMAT_R16G16B16A16_SFLOAT;
                // 32-bit
            case RAL::Format::R32_SFLOAT:           return VK_FORMAT_R32_SFLOAT;
            case RAL::Format::R32G32_SFLOAT:        return VK_FORMAT_R32G32_SFLOAT;
            case RAL::Format::R32G32B32_SFLOAT:     return VK_FORMAT_R32G32B32_SFLOAT;
            case RAL::Format::R32G32B32A32_SFLOAT:  return VK_FORMAT_R32G32B32A32_SFLOAT;
            case RAL::Format::R32_UINT:             return VK_FORMAT_R32_UINT;
            case RAL::Format::R32G32_UINT:          return VK_FORMAT_R32G32_UINT;
            case RAL::Format::R32G32B32_UINT:       return VK_FORMAT_R32G32B32_UINT;
            case RAL::Format::R32G32B32A32_UINT:    return VK_FORMAT_R32G32B32A32_UINT;
                // Depth
            case RAL::Format::D32_SFLOAT:           return VK_FORMAT_D32_SFLOAT;
            case RAL::Format::D24_UNORM_S8_UINT:    return VK_FORMAT_D24_UNORM_S8_UINT;
            case RAL::Format::D32_SFLOAT_S8_UINT:   return VK_FORMAT_D32_SFLOAT_S8_UINT;
                // Block Compression
            case RAL::Format::BC1_RGB_UNORM:        return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            case RAL::Format::BC3_UNORM:            return VK_FORMAT_BC3_UNORM_BLOCK;
            case RAL::Format::BC7_UNORM:            return VK_FORMAT_BC7_UNORM_BLOCK;

            case RAL::Format::UNKNOWN:
            default:
                // You should have a logging system here
                // For now, we'll assert or throw
                throw std::runtime_error("Unsupported or unknown RAL::Format!");
                return VK_FORMAT_UNDEFINED;
        }
    }


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
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    // Choose the best settings from the available options
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat: availableFormats) {
            // We want sRGB color space for better visuals
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0]; // Fallback to the first available format
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes, bool vsync) {
        if (!vsync) {
            for (const auto &availablePresentMode: availablePresentModes) {
                // Prefer triple buffering for low latency without tearing
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return availablePresentMode;
                }
            }
        }
        // Guaranteed to be available, standard VSync
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

    VulkanDevice::VulkanDevice(GLFWwindow *window) {
        // === 1. Create VkInstance ===
        // The instance is the connection between your application and the Vulkan library.
        {
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Helios Engine";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "Helios";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_2;

            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            // Get required extensions from GLFW
            uint32_t glfwExtensionCount = 0;
            const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            // Add the debug messenger extension
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            // Enable validation layers - this is non-negotiable for development
            const std::vector<const char *> validationLayers = {
                    "VK_LAYER_KHRONOS_validation"
            };
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
        }

        // === 2. Setup Debug Messenger ===
        // This will give us validation layer messages in our console.
        {
            VkDebugUtilsMessengerCreateInfoEXT createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType =
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = DebugCallback;

            // We have to load the function pointer for this extension function ourselves
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance,
                                                                                   "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr) {
                VK_CHECK(func(m_Instance, &createInfo, nullptr, &m_DebugMessenger));
            } else {
                // Handle error
            }
        }

        // === 3. Create Window Surface ===
        // This is the bridge between Vulkan and the OS window manager, handled by GLFW.
        VK_CHECK(glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface));

        // === 4. Pick Physical Device (GPU) ===
        // We'll simplify this for now and just pick the first available discrete GPU.
        // A real engine would have a scoring system to pick the best device.
        {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

            m_PhysicalDevice = devices[0]; // TODO: Implement proper device picking

            // Find a queue family that supports graphics operations.
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

            for (uint32_t i = 0; i < queueFamilyCount; ++i) {
                if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    m_GraphicsQueueFamilyIndex = i;
                    break;
                }
            }
        }

        // === 5. Create Logical Device & Queues ===
        // The logical device is our interface to the physical device.
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
            queueCreateInfo.queueCount = 1;
            float queuePriority = 1.0f;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            VkPhysicalDeviceFeatures deviceFeatures{}; // Enable features here if needed

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pQueueCreateInfos = &queueCreateInfo;
            createInfo.queueCreateInfoCount = 1;
            createInfo.pEnabledFeatures = &deviceFeatures;

            // Enable the mandatory swapchain extension
            const std::vector<const char *> deviceExtensions = {
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };
            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            VK_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice));

            // Retrieve the handle to the queue
            vkGetDeviceQueue(m_LogicalDevice, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
            // In a more complex scenario, the present queue might be different from the graphics queue.
            // For now, we assume they are the same.
            m_PresentQueue = m_GraphicsQueue;
        }

        // === 6. Create Swapchain ===
        {
            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Create it in the signaled state so the first frame doesn't wait forever

            VK_CHECK(vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore));
            VK_CHECK(vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore));
            VK_CHECK(vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &m_InFlightFence));
        }

        RDE_CORE_INFO("Vulkan Device Initialized successfully.");
    }

    VulkanDevice::~VulkanDevice() {
        wait_idle(); // Ensure GPU is not busy before we start destroying things
        // Destruction must happen in reverse order of creation.

        destroy_swapchain();

        vkDestroySemaphore(m_LogicalDevice, m_RenderFinishedSemaphore, nullptr);
        vkDestroySemaphore(m_LogicalDevice, m_ImageAvailableSemaphore, nullptr);
        vkDestroyFence(m_LogicalDevice, m_InFlightFence, nullptr);

        vkDestroyDevice(m_LogicalDevice, nullptr);

        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance,
                                                                                "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
    }

    void VulkanDevice::create_swapchain(const RAL::SwapchainDescription &desc) {
        // Query support and choose settings
        SwapchainSupportDetails support = querySwapchainSupport(m_PhysicalDevice, m_Surface);
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(support.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(support.presentModes, desc.vsync);
        VkExtent2D extent = chooseSwapExtent(support.capabilities, m_Window);

        uint32_t imageCount = support.capabilities.minImageCount + 1;
        if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount) {
            imageCount = support.capabilities.maxImageCount;
        }

        // Create the swapchain
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // We will render directly to it

        // We assume graphics and present queues are the same for simplicity
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = support.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VK_CHECK(vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_Swapchain.handle));

        // Store swapchain properties
        m_Swapchain.imageFormat = surfaceFormat.format;
        m_Swapchain.extent = extent;

        // Retrieve image handles
        vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain.handle, &imageCount, nullptr);
        m_Swapchain.images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain.handle, &imageCount, m_Swapchain.images.data());

        // Create image views
        m_Swapchain.imageViews.resize(imageCount);
        for (size_t i = 0; i < m_Swapchain.images.size(); i++) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_Swapchain.images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_Swapchain.imageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VK_CHECK(vkCreateImageView(m_LogicalDevice, &viewInfo, nullptr, &m_Swapchain.imageViews[i]));
        }

        // --- Create RAL TextureHandles for the swapchain images ---
        // NOTE: This is a placeholder. A real resource manager would be responsible
        // for creating these handles and associating them with the VkImageViews.
        // For now, we are creating "unmanaged" handles. The rest of the engine
        // doesn't know the difference, which proves the power of abstraction.
        m_SwapchainTextureHandles.resize(m_Swapchain.imageViews.size());
        for (size_t i = 0; i < m_Swapchain.imageViews.size(); ++i) {
            // We'll just use the index as the handle index for simplicity.
            // A real manager would have a free list and handle generations.
            m_SwapchainTextureHandles[i] = RAL::TextureHandle{static_cast<uint32_t>(i), 1};
        }
    }

    void VulkanDevice::destroy_swapchain() {
        // Wait until the device is idle before destroying a swapchain,
        // as its images may still be in use.
        wait_idle();

        for (auto imageView: m_Swapchain.imageViews) {
            vkDestroyImageView(m_LogicalDevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain.handle, nullptr);

        m_Swapchain.handle = VK_NULL_HANDLE;
        m_Swapchain.imageViews.clear();
        m_Swapchain.images.clear();
    }

    RAL::TextureHandle VulkanDevice::acquire_next_swapchain_image() {
        // 1. Wait for the previous frame to finish.
        // This fence ensures that we don't start a new frame while the GPU is still
        // working on one from two frames ago. It limits frames in flight to 1.
        // The timeout (UINT64_MAX) means we wait indefinitely.
        vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);

        vkResetCommandPool(m_LogicalDevice, m_CommandPool, 0);

        vkResetFences(m_LogicalDevice, 1, &m_InFlightFence);

        // 2. Acquire an image from the swapchain.
        // This call will signal m_ImageAvailableSemaphore when the image is ready.
        VkResult result = vkAcquireNextImageKHR(
                m_LogicalDevice,
                m_Swapchain.handle,
                UINT64_MAX,
                m_ImageAvailableSemaphore, // Semaphore to be signaled
                VK_NULL_HANDLE,            // Fence to be signaled (we're using our own)
                &m_CurrentImageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // Swapchain is out of date (e.g., window was resized).
            // We need to recreate it. For now, we'll return an invalid handle.
            // A full application would trigger a recreation here.
            return RAL::TextureHandle::INVALID();
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            // Handle other errors
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        // 3. Return the abstract handle for the acquired image.
        // The rest of the engine can now use this handle to build a render pass.
        return m_SwapchainTextureHandles[m_CurrentImageIndex];
    }

    // NOTE: This is a simplified Submit for a single command buffer.
// The interface takes a vector, but we'll implement for the common case of one.
    void VulkanDevice::submit(const std::vector<std::unique_ptr<RAL::CommandBuffer>> &command_buffers) {
        if (command_buffers.empty()) {
            return;
        }

        // For now, we assume one command buffer per submission.
        // We need to downcast from our interface to the concrete Vulkan implementation.
        VulkanCommandBuffer *vulkanCmd = static_cast<VulkanCommandBuffer *>(command_buffers[0].get());
        VkCommandBuffer cmdBuffer = vulkanCmd->get_handle(); // Assumes this function exists on VulkanCommandBuffer

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // Wait on the m_ImageAvailableSemaphore before executing the command buffer.
        // We only want to start writing to the color attachment when the image is ready.
        VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        // The command buffer to execute.
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        // Signal the m_RenderFinishedSemaphore when the command buffer has finished execution.
        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // Submit to the graphics queue.
        // The m_InFlightFence will be signaled when this submission is complete.
        VK_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFence));
    }

    void VulkanDevice::present() {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // Wait on the m_RenderFinishedSemaphore. This ensures presentation doesn't happen
        // until rendering is complete.
        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphore};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        // Specify the swapchain and image index to present.
        VkSwapchainKHR swapchains[] = {m_Swapchain.handle};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &m_CurrentImageIndex;

        VkResult result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // Swapchain is suboptimal. A robust app would flag the swapchain for recreation
            // on the next frame.
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swapchain image!");
        }
    }

    uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory type!");
    }

    void VulkanDevice::createVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                          VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK(vkCreateBuffer(m_LogicalDevice, &bufferInfo, nullptr, &buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_LogicalDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
        VK_CHECK(vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &bufferMemory));

        vkBindBufferMemory(m_LogicalDevice, buffer, bufferMemory, 0);
    }

    void VulkanDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        // This is a simplified copy command that should be done on a dedicated transfer queue
        // in a real engine. For now, we use the graphics queue.
        std::unique_ptr<RAL::CommandBuffer> cmd = create_command_buffer();
        cmd->begin();

        // The cast is ugly, but necessary here. A better design might expose the VkCommandBuffer handle.
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(static_cast<VulkanCommandBuffer *>(cmd.get())->get_handle(), srcBuffer, dstBuffer, 1,
                        &copyRegion);

        cmd->end();

        // Submit and wait for completion. This is synchronous and inefficient, but simple.
        std::vector<std::unique_ptr<RAL::CommandBuffer>> submissions;
        submissions.push_back(std::move(cmd));
        submit(submissions);
        wait_idle(); // Extremely inefficient! For learning purposes only.
    }

    VkShaderModule VulkanDevice::createShaderModule(const std::vector<char> &code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        VK_CHECK(vkCreateShaderModule(m_LogicalDevice, &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }

    // Implement the public interface function
    RAL::ShaderHandle VulkanDevice::create_shader(const RAL::ShaderDescription &desc) {
        auto shaderCode = FileIO::ReadFile(desc.filePath);
        VulkanShader newShader;
        newShader.module = createShaderModule(shaderCode);
        return m_ShaderManager.create(std::move(newShader));
    }

    void VulkanDevice::destroy_shader(RAL::ShaderHandle handle) {
        if (m_ShaderManager.is_valid(handle)) {
            wait_idle();
            VulkanShader &shader = m_ShaderManager.get(handle);
            vkDestroyShaderModule(m_LogicalDevice, shader.module, nullptr);
            m_ShaderManager.destroy(handle);
        }
    }

    RAL::PipelineHandle VulkanDevice::create_pipeline(const RAL::PipelineDescription &desc) {
        VulkanPipeline newPipeline;

        // --- 1. Shader Stages ---
        VulkanShader &vs = m_ShaderManager.get(desc.vertexShader);
        VulkanShader &fs = m_ShaderManager.get(desc.fragmentShader);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vs.module;
        vertShaderStageInfo.pName = "main"; // Entry point

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fs.module;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        // --- 2. Vertex Input ---
        std::vector<VkVertexInputBindingDescription> bindings;
        for (const auto &b: desc.vertexBindings) {
            bindings.push_back({.binding = b.binding, .stride = b.stride, .inputRate = VK_VERTEX_INPUT_RATE_VERTEX});
        }
        std::vector<VkVertexInputAttributeDescription> attributes;
        for (const auto &a: desc.vertexAttributes) {
            attributes.push_back({.location = a.location, .binding = a.binding, .format = ToVulkanFormat(
                    a.format), .offset = a.offset});
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
        vertexInputInfo.pVertexBindingDescriptions = bindings.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

        // --- 3. Fixed Function Stages (with sensible defaults) ---
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1; // Viewport and scissor will be set dynamically
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE; // No blending for our first triangle

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        // --- 4. Pipeline Layout ---
        // Describes uniform buffers, textures, etc. For now, it's empty.
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        VK_CHECK(vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayoutInfo, nullptr, &newPipeline.layout));

        // Set dynamic states that can be changed without recreating the pipeline
        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // --- 5. Create the Pipeline ---
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // No depth testing for now
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = newPipeline.layout;
        pipelineInfo.renderPass = m_SwapchainRenderPass; // Must be compatible with this render pass
        pipelineInfo.subpass = 0;

        VK_CHECK(vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                           &newPipeline.handle));

        return m_PipelineManager.create(std::move(newPipeline));
    }

    void VulkanDevice::destroy_pipeline(RAL::PipelineHandle handle) {
        if (m_PipelineManager.is_valid(handle)) {
            wait_idle();
            VulkanPipeline &pipeline = m_PipelineManager.get(handle);
            vkDestroyPipeline(m_LogicalDevice, pipeline.handle, nullptr);
            vkDestroyPipelineLayout(m_LogicalDevice, pipeline.layout, nullptr);
            m_PipelineManager.destroy(handle);
        }
    }

    RAL::CommandBuffer *VulkanDevice::begin_frame(){
        assert(m_CurrentFrameCommandBuffer == nullptr && "begin_frame() called twice without a call to end_frame()!");
        // Acquire returns the handle, but we don't need it here. The command buffer
        // will get it from the device when BeginRenderPass is called.
        // We just need to know if the acquisition was successful.
        RAL::TextureHandle swapchain_image = acquire_next_swapchain_image();
        if (!swapchain_image.is_valid()) {
            // Window was resized or another error occurred.
            return nullptr;
        }

        // The user gets a raw pointer, but we return a new object each frame.
        // This is a bit tricky for ownership. A better approach is to return
        // a single, reusable command buffer owned by the device.
        // For now, let's assume we create a new one.
        m_CurrentFrameCommandBuffer = create_command_buffer();
        m_CurrentFrameCommandBuffer->begin();
        return m_CurrentFrameCommandBuffer.get();
    }

    void VulkanDevice::end_frame() {
        assert(m_CurrentFrameCommandBuffer != nullptr && "end_frame() called without a call to begin_frame()!");

        m_CurrentFrameCommandBuffer->end();

        std::vector<std::unique_ptr<RAL::CommandBuffer>> submissions;
        submissions.push_back(std::move(m_CurrentFrameCommandBuffer));

        submit(submissions);
        present();
    }

    // ... Stubs for other functions ...
    void VulkanDevice::wait_idle() {
        vkDeviceWaitIdle(m_LogicalDevice);
    }

}