#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommon.h"
#include "VulkanMappers.h"

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
            appInfo.pApplicationName = "RDEngine";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "RDEngine";
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

            vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties);
            RDE_CORE_INFO("Selected GPU: {}", m_PhysicalDeviceProperties.deviceName);
            RDE_CORE_INFO("Max Sampler Anisotropy: {}", m_PhysicalDeviceProperties.limits.maxSamplerAnisotropy);

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
            // Check if the GPU supports samplerAnisotropy before enabling it
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &supportedFeatures);
            if (supportedFeatures.samplerAnisotropy) {
                deviceFeatures.samplerAnisotropy = VK_TRUE;
            } else {
                RDE_CORE_WARN("Sampler Anisotropy is not supported on this device!");
            }

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

        // === 6. Create Command Pool ===
        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;

            // This flag is crucial. It allows us to reset individual command buffers,
            // which is essential for re-recording them each frame.
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            VK_CHECK(vkCreateCommandPool(m_LogicalDevice, &poolInfo, nullptr, &m_CommandPool));
        }

        // === 7. Create Upload Context ===
        {
            // Create a separate command pool for upload commands
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
            // TRANSIENT_BIT tells the driver that these command buffers will be short-lived,
            // which can be a performance optimization.
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            VK_CHECK(vkCreateCommandPool(m_LogicalDevice, &poolInfo, nullptr, &m_UploadCommandPool));

            // Allocate the command buffer
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = m_UploadCommandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            VK_CHECK(vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &m_UploadCommandBuffer));

            // Create the fence
            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            VK_CHECK(vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &m_UploadFence));
        }

        // === 6. Create Swapchain ===
        {
            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            // Create it in the signaled state so the first frame doesn't wait forever

            VK_CHECK(vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore));
            VK_CHECK(vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore));
            VK_CHECK(vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &m_InFlightFence));
        }
        {
            std::vector<VkDescriptorPoolSize> poolSizes = {
                    {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000}
                    // Add other types as you need them
            };
            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // Allows freeing individual sets
            poolInfo.maxSets = 1000 * poolSizes.size();
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();

            VK_CHECK(vkCreateDescriptorPool(m_LogicalDevice, &poolInfo, nullptr, &m_DescriptorPool));
        }
        {
            VmaAllocatorCreateInfo allocatorInfo = {};
            allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2; // Or your target version
            allocatorInfo.physicalDevice = m_PhysicalDevice;
            allocatorInfo.device = m_LogicalDevice;
            allocatorInfo.instance = m_Instance;

            VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_VmaAllocator));
        }

        RDE_CORE_INFO("Vulkan Device Initialized successfully.");
    }

    VulkanDevice::~VulkanDevice() {
        wait_idle(); // Ensure GPU is not busy before we start destroying things
        // Destruction must happen in reverse order of creation.
        vkDestroyDescriptorPool(m_LogicalDevice, m_DescriptorPool, nullptr);
        vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
        destroy_swapchain();

        vkDestroySemaphore(m_LogicalDevice, m_RenderFinishedSemaphore, nullptr);
        vkDestroySemaphore(m_LogicalDevice, m_ImageAvailableSemaphore, nullptr);
        vkDestroyFence(m_LogicalDevice, m_InFlightFence, nullptr);

        vmaDestroyAllocator(m_VmaAllocator);

        vkDestroyFence(m_LogicalDevice, m_UploadFence, nullptr);
        vkDestroyCommandPool(m_LogicalDevice, m_UploadCommandPool,
                             nullptr); // The command buffer is freed with the pool

        vkDestroyDevice(m_LogicalDevice, nullptr);

        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance,
                                                                                "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
    }

    void *VulkanDevice::map_buffer(RAL::BufferHandle handle) {
        if (!m_BufferManager.is_valid(handle)) {
            return nullptr;
        }
        VulkanBuffer &buffer = m_BufferManager.get(handle);

        void *mappedData = nullptr;
        // vmaMapMemory works on the VmaAllocation handle.
        // It will fail internally if the memory type is not mappable.
        VkResult result = vmaMapMemory(m_VmaAllocator, buffer.allocation, &mappedData);

        if (result != VK_SUCCESS) {
            //TODO map to string
            /*RDE_CORE_ERROR("VMA: Failed to map buffer! Error: {}", result);*/
            return nullptr;
        }

        return mappedData;
    }

    void VulkanDevice::unmap_buffer(RAL::BufferHandle handle) {
        if (!m_BufferManager.is_valid(handle)) {
            return;
        }
        VulkanBuffer &buffer = m_BufferManager.get(handle);

        // vmaUnmapMemory also works on the VmaAllocation handle.
        // If the memory is coherent, VMA handles flushing automatically if needed.
        vmaUnmapMemory(m_VmaAllocator, buffer.allocation);
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

        if (imageCount < 3) imageCount = 3;

        // Check against max again in case our desired count (3) is too high.
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
        // --- Create the Render Pass ---
        {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = m_Swapchain.imageFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear the framebuffer before rendering
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store the result
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // We don't care about the previous layout
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Layout must be ready for presentation

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;

            // Add a subpass dependency to ensure layout transition happens at the right time
            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &colorAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;

            VK_CHECK(vkCreateRenderPass(m_LogicalDevice, &renderPassInfo, nullptr, &m_SwapchainRenderPass));
        }

        // --- Create Framebuffers ---
        {
            m_SwapchainFramebuffers.resize(m_Swapchain.imageViews.size());
            for (size_t i = 0; i < m_Swapchain.imageViews.size(); i++) {
                VkImageView attachments[] = {
                        m_Swapchain.imageViews[i]
                };

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = m_SwapchainRenderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachments;
                framebufferInfo.width = m_Swapchain.extent.width;
                framebufferInfo.height = m_Swapchain.extent.height;
                framebufferInfo.layers = 1;

                VK_CHECK(vkCreateFramebuffer(m_LogicalDevice, &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]));
            }
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

        for (auto framebuffer: m_SwapchainFramebuffers) {
            vkDestroyFramebuffer(m_LogicalDevice, framebuffer, nullptr);
        }
        vkDestroyRenderPass(m_LogicalDevice, m_SwapchainRenderPass, nullptr);
        m_SwapchainFramebuffers.clear();
        m_SwapchainRenderPass = VK_NULL_HANDLE;

        for (auto imageView: m_Swapchain.imageViews) {
            vkDestroyImageView(m_LogicalDevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain.handle, nullptr);

        m_Swapchain.handle = VK_NULL_HANDLE;
        m_Swapchain.imageViews.clear();
        m_Swapchain.images.clear();
        m_SwapchainTextureHandles.clear();
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
                VK_NULL_HANDLE, // Fence to be signaled (we're using our own)
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

    std::unique_ptr<RAL::CommandBuffer> VulkanDevice::create_command_buffer() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer vkCommandBuffer;
        VK_CHECK(vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &vkCommandBuffer));

        // Wrap the native Vulkan handle in our C++ class, passing a pointer
        // to this device so the command buffer can access it.
        return std::make_unique<VulkanCommandBuffer>(vkCommandBuffer, this);
    }

    // NOTE: This is a simplified Submit for a single command buffer.
    // The interface takes a vector, but we'll implement for the common case of one.
    void VulkanDevice::submit(const std::vector<std::unique_ptr<RAL::CommandBuffer> > &command_buffers) {
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

    void VulkanDevice::immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function) {
        // Begin the command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(m_UploadCommandBuffer, &beginInfo));

        // Execute the provided command recording function
        function(m_UploadCommandBuffer);

        // End the command buffer
        VK_CHECK(vkEndCommandBuffer(m_UploadCommandBuffer));

        // Submit the command buffer
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_UploadCommandBuffer;
        VK_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_UploadFence));

        // Wait for the fence to signal that the command has finished executing
        VK_CHECK(vkWaitForFences(m_LogicalDevice, 1, &m_UploadFence, VK_TRUE, UINT64_MAX));
        // Reset the fence for the next submission
        VK_CHECK(vkResetFences(m_LogicalDevice, 1, &m_UploadFence));

        // Reset the command pool to release the command buffer's memory.
        // This is faster than freeing/re-allocating the command buffer itself.
        VK_CHECK(vkResetCommandPool(m_LogicalDevice, m_UploadCommandPool, 0));
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

    void VulkanDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        immediate_submit([&](VkCommandBuffer cmd) {
            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
        });
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
        vertShaderStageInfo.pName = "main";

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

        // --- 3. Fixed Function Stages ---
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Viewport and Scissor will be dynamic, so we just need to specify the count.
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // Dynamic states allow us to change viewport and scissor without rebuilding the pipeline
        std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Map from desc.rasterizationState later
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;   // Map from desc.rasterizationState later
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // Map from desc.rasterizationState later
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        // For ImGui, you need blending enabled.
        // We should map this from desc.colorBlendState properly.
        colorBlendAttachment.blendEnable = desc.colorBlendState.attachment.blendEnable; // VK_TRUE for ImGui
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        // --- 4. Pipeline Layout ---
        std::vector<VkDescriptorSetLayout> vkSetLayouts;
        for (const auto &layoutHandle: desc.descriptorSetLayouts) {
            vkSetLayouts.push_back(m_DsLayoutManager.get(layoutHandle).handle);
        }
        std::vector<VkPushConstantRange> vkPushRanges;
        for (const auto &pushRange: desc.pushConstantRanges) {
            vkPushRanges.push_back({ToVulkanShaderStageFlags(pushRange.stages), pushRange.offset, pushRange.size});
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(vkSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = vkSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(vkPushRanges.size());
        pipelineLayoutInfo.pPushConstantRanges = vkPushRanges.data();
        VK_CHECK(vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayoutInfo, nullptr, &newPipeline.layout));

        // --- 5. Create the Graphics Pipeline ---
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2; // Vertex + Fragment
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
        // THIS IS CRUCIAL: The pipeline must know which render pass it's compatible with.
        pipelineInfo.renderPass = m_SwapchainRenderPass;
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

    RAL::DescriptorSetLayoutHandle VulkanDevice::create_descriptor_set_layout(
            const RAL::DescriptorSetLayoutDescription &desc) {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(desc.bindings.size());

        for (const auto &ralBinding: desc.bindings) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = ralBinding.binding;
            layoutBinding.descriptorType = ToVulkanDescriptorType(ralBinding.type);
            layoutBinding.descriptorCount = 1; // Not supporting arrays of resources yet
            layoutBinding.stageFlags = ToVulkanShaderStageFlags(ralBinding.stages);
            layoutBinding.pImmutableSamplers = nullptr;
            bindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VulkanDescriptorSetLayout layout;
        VK_CHECK(vkCreateDescriptorSetLayout(m_LogicalDevice, &layoutInfo, nullptr, &layout.handle));

        return m_DsLayoutManager.create(std::move(layout));
    }

    void VulkanDevice::destroy_descriptor_set_layout(RAL::DescriptorSetLayoutHandle handle) {
        if (m_DsLayoutManager.is_valid(handle)) {
            wait_idle();
            auto layout = m_DsLayoutManager.get(handle);
            vkDestroyDescriptorSetLayout(m_LogicalDevice, layout.handle, nullptr);
            m_DsLayoutManager.destroy(handle);
        }
    }

    // Note: Descriptor sets are often allocated from a pool for efficiency.
    // This simple create function is a good starting point.
    RAL::DescriptorSetHandle VulkanDevice::create_descriptor_set(const RAL::DescriptorSetDescription &desc) {
        // Get the concrete Vulkan layout handle
        auto layout = m_DsLayoutManager.get(desc.layout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout.handle;


        VkDescriptorSet newSet;
        VK_CHECK(vkAllocateDescriptorSets(m_LogicalDevice, &allocInfo, &newSet));

        // --- Write the actual resource handles into the set ---
        std::vector<VkWriteDescriptorSet> descriptorWrites;

        // These must live until vkUpdateDescriptorSets is called
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkDescriptorImageInfo> imageInfos;

        for (const auto &ralWrite: desc.writes) {
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = newSet;
            write.dstBinding = ralWrite.binding;
            write.dstArrayElement = 0;
            write.descriptorType = ToVulkanDescriptorType(ralWrite.type);
            write.descriptorCount = 1;

            switch (ralWrite.type) {
                case RAL::DescriptorType::UniformBuffer: {
                    // Get the concrete buffer and create the info struct
                    VulkanBuffer &buffer = m_BufferManager.get(ralWrite.buffer);
                    bufferInfos.push_back({buffer.handle, 0, VK_WHOLE_SIZE});
                    write.pBufferInfo = &bufferInfos.back();
                    break;
                }
                case RAL::DescriptorType::CombinedImageSampler: {
                    // Get the concrete texture view and sampler
                    VulkanTexture &texture = m_TextureManager.get(ralWrite.texture);
                    VulkanSampler sampler = m_SamplerManager.get(ralWrite.sampler);
                    imageInfos.push_back({
                                                 sampler.handle, texture.image_view,
                                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                         });
                    write.pImageInfo = &imageInfos.back();
                    break;
                }
            }
            descriptorWrites.push_back(write);
        }

        vkUpdateDescriptorSets(m_LogicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
                               0, nullptr);

        return m_DescriptorSetManager.create(std::move(newSet));
    }

    void VulkanDevice::destroy_descriptor_set(RAL::DescriptorSetHandle handle) {
        if (m_DescriptorSetManager.is_valid(handle)) {
            wait_idle();
            auto set = m_DescriptorSetManager.get(handle);
            // We FREE the set back to the pool, we don't destroy it.
            vkFreeDescriptorSets(m_LogicalDevice, m_DescriptorPool, 1, &set);
            m_DescriptorSetManager.destroy(handle); // Reclaim the handle slot
        }
    }

    RAL::SamplerHandle VulkanDevice::create_sampler(const RAL::SamplerDescription &desc) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = ToVulkanFilter(desc.magFilter);
        samplerInfo.minFilter = ToVulkanFilter(desc.minFilter);
        samplerInfo.addressModeU = ToVulkanAddressMode(desc.addressModeU);
        samplerInfo.addressModeV = ToVulkanAddressMode(desc.addressModeV);
        samplerInfo.addressModeW = ToVulkanAddressMode(desc.addressModeW);
        samplerInfo.anisotropyEnable = VK_TRUE; // Good default
        samplerInfo.maxAnisotropy = m_PhysicalDeviceProperties.limits.maxSamplerAnisotropy; // Use max supported
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

        VulkanSampler sampler;
        VK_CHECK(vkCreateSampler(m_LogicalDevice, &samplerInfo, nullptr, &sampler.handle));

        return m_SamplerManager.create(std::move(sampler));
    }

    void VulkanDevice::destroy_sampler(RAL::SamplerHandle handle) {
        if (m_SamplerManager.is_valid(handle)) {
            // Defer this! For now, wait idle.
            wait_idle();
            auto sampler = m_SamplerManager.get(handle);
            vkDestroySampler(m_LogicalDevice, sampler.handle, nullptr);
            m_SamplerManager.destroy(handle);
        }
    }

    RAL::BufferHandle VulkanDevice::create_buffer(const RAL::BufferDescription &desc) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = desc.size;
        bufferInfo.usage = ToVulkanBufferUsage(desc.usage);

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = ToVmaMemoryUsage(desc.memoryUsage);

        if (desc.initialData && desc.memoryUsage == RAL::MemoryUsage::DeviceLocal) {
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        VulkanBuffer newBuffer;
        VK_CHECK(
                vmaCreateBuffer(m_VmaAllocator, &bufferInfo, &allocInfo, &newBuffer.handle, &newBuffer.allocation,
                                nullptr));

        if (desc.initialData) {
            if (desc.memoryUsage == RAL::MemoryUsage::HostVisible) {
                void *mappedData;
                vmaMapMemory(m_VmaAllocator, newBuffer.allocation, &mappedData);
                memcpy(mappedData, desc.initialData, desc.size);
                vmaUnmapMemory(m_VmaAllocator, newBuffer.allocation);
            } else {
                // Use a staging buffer for device-local memory
                VulkanBuffer stagingBuffer;

                VkBufferCreateInfo stagingBufferInfo{};
                stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                stagingBufferInfo.size = desc.size;
                stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

                VmaAllocationCreateInfo stagingAllocInfo = {};
                stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; // Staging buffer is on the CPU side
                stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

                VK_CHECK(
                        vmaCreateBuffer(m_VmaAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer.handle, &
                                stagingBuffer.allocation, nullptr));

                void *mappedData;
                vmaMapMemory(m_VmaAllocator, stagingBuffer.allocation, &mappedData);
                memcpy(mappedData, desc.initialData, desc.size);
                vmaUnmapMemory(m_VmaAllocator, stagingBuffer.allocation);

                copyBuffer(stagingBuffer.handle, newBuffer.handle, desc.size);

                vmaDestroyBuffer(m_VmaAllocator, stagingBuffer.handle, stagingBuffer.allocation);
            }
        }

        return m_BufferManager.create(std::move(newBuffer));
    }

    void VulkanDevice::destroy_buffer(RAL::BufferHandle handle) {
        if (m_BufferManager.is_valid(handle)) {
            wait_idle();
            VulkanBuffer &buffer = m_BufferManager.get(handle);
            // Single call to destroy buffer and free memory
            vmaDestroyBuffer(m_VmaAllocator, buffer.handle, buffer.allocation);
            m_BufferManager.destroy(handle);
        }
    }

    RAL::TextureHandle VulkanDevice::create_texture(const RAL::TextureDescription &desc) {
        VulkanTexture newTexture;

        // 1. Create the VkImage
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D; // Expand later for 1D/3D/Cube
        imageInfo.extent.width = desc.width;
        imageInfo.extent.height = desc.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = desc.mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = ToVulkanFormat(desc.format);
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // Always use optimal for performance
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = ToVulkanImageUsage(desc.usage);
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // Add transfer destination usage if we need to upload data
        if (desc.initialData) {
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        VK_CHECK(vmaCreateImage(m_VmaAllocator, &imageInfo, &allocInfo, &newTexture.handle, &newTexture.allocation,
                                nullptr));

        // 2. Handle Initial Data Upload (if provided)
        if (desc.initialData) {
            // Create the CPU-visible staging buffer
            VulkanBuffer stagingBuffer;
            VkBufferCreateInfo stagingBufferInfo{};
            stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingBufferInfo.size = desc.initialDataSize; // Using the size from the description
            stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            VmaAllocationCreateInfo stagingAllocInfo = {};
            stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

            VK_CHECK(vmaCreateBuffer(m_VmaAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer.handle,
                                     &stagingBuffer.allocation, nullptr));

            // Copy data from the application to the staging buffer
            void *mappedData;
            vmaMapMemory(m_VmaAllocator, stagingBuffer.allocation, &mappedData);
            memcpy(mappedData, desc.initialData, static_cast<size_t>(desc.initialDataSize));
            vmaUnmapMemory(m_VmaAllocator, stagingBuffer.allocation);

            // --- USE immediate_submit TO PERFORM THE GPU-SIDE COPY AND LAYOUT TRANSITIONS ---
            immediate_submit([&](VkCommandBuffer cmd) {
                // 1. Transition image layout to be ready for copying
                VkImageMemoryBarrier barrier_to_transfer{};
                barrier_to_transfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier_to_transfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier_to_transfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier_to_transfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier_to_transfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier_to_transfer.image = newTexture.handle;
                barrier_to_transfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier_to_transfer.subresourceRange.baseMipLevel = 0;
                barrier_to_transfer.subresourceRange.levelCount = 1;
                barrier_to_transfer.subresourceRange.baseArrayLayer = 0;
                barrier_to_transfer.subresourceRange.layerCount = 1;
                barrier_to_transfer.srcAccessMask = 0;
                barrier_to_transfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                                     nullptr, 0, nullptr, 1, &barrier_to_transfer);

                // 2. Copy the data from the staging buffer to the GPU-local image
                VkBufferImageCopy region{};
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = {0, 0, 0};
                region.imageExtent = {desc.width, desc.height, 1};
                vkCmdCopyBufferToImage(cmd, stagingBuffer.handle, newTexture.handle,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

                // 3. Transition image layout to be ready for shader reading
                VkImageMemoryBarrier barrier_to_shader_read{};
                barrier_to_shader_read.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier_to_shader_read.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier_to_shader_read.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier_to_shader_read.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier_to_shader_read.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier_to_shader_read.image = newTexture.handle;
                barrier_to_shader_read.subresourceRange = barrier_to_transfer.subresourceRange; // Subresource range is the same
                barrier_to_shader_read.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier_to_shader_read.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                                     nullptr, 0, nullptr, 1, &barrier_to_shader_read);
            });

            // Clean up the temporary staging buffer now that the data is on the GPU
            vmaDestroyBuffer(m_VmaAllocator, stagingBuffer.handle, stagingBuffer.allocation);
        }

        // 3. Create the VkImageView
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = newTexture.handle;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = ToVulkanFormat(desc.format);
        // Note: Use an appropriate aspect mask for depth/stencil images
        if (imageInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        } else {
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(m_LogicalDevice, &viewInfo, nullptr, &newTexture.image_view));

        return m_TextureManager.create(std::move(newTexture));
    }

    void VulkanDevice::destroy_texture(RAL::TextureHandle handle) {
        if (m_TextureManager.is_valid(handle)) {
            wait_idle(); // Simple sync
            VulkanTexture &texture = m_TextureManager.get(handle);

            // Must destroy the view before the image
            vkDestroyImageView(m_LogicalDevice, texture.image_view, nullptr);
            vmaDestroyImage(m_VmaAllocator, texture.handle, texture.allocation);

            m_TextureManager.destroy(handle);
        }
    }

    RAL::CommandBuffer *VulkanDevice::begin_frame() {
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

        std::vector<std::unique_ptr<RAL::CommandBuffer> > submissions;
        submissions.push_back(std::move(m_CurrentFrameCommandBuffer));

        submit(submissions);
        present();
    }

    // ... Stubs for other functions ...
    void VulkanDevice::wait_idle() {
        vkDeviceWaitIdle(m_LogicalDevice);
    }
}
