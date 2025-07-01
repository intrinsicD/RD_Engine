#include "VulkanDevice.h"
#include "VulkanMappers.h"
#include "VulkanCommandBuffer.h"
#include "Log.h"

#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <memory>

// Place the VMA implementation macro here
#define VMA_IMPLEMENTATION

#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

namespace RDE {

    struct VulkanDevice::PImpl {
        // We will store our concrete resources here. The key is the ID from the RAL handle.
        std::unordered_map<uint64_t, VulkanBuffer> buffers;
        std::unordered_map<uint64_t, VulkanTexture> textures;

        // A simple counter to generate unique handle IDs.
        uint64_t next_handle_id = 1;
    };

    VulkanDevice::VulkanDevice() {
        m_pimpl = std::make_unique<PImpl>();
        // 1. Create Vulkan Instance
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "RDE Engine";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "RDE";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        // For now, enable minimal required extensions. A real engine queries for support.
        // We need a surface extension to draw to a window eventually.
        std::vector<const char *> extensions = {
                "VK_KHR_surface", /* platform specific surface, e.g., "VK_KHR_win32_surface" */
                "VK_KHR_xcb_surface", // Add this for X11 support
        };
        // Add debug utils extension for validation layers
        const char *validation_layer = "VK_LAYER_KHRONOS_validation";
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = &validation_layer;

        if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
        RDE_CORE_INFO("Vulkan Instance created successfully.");

        // 2. Pick Physical Device
        pick_physical_device();

        // 3. Create Logical Device
        create_logical_device();

        // 4. Create VMA Allocator
        create_allocator();
    }

    VulkanDevice::~VulkanDevice() {
        // Ensure all commands are finished before we start tearing things down.
        if (m_device) {
            vkDeviceWaitIdle(m_device);
        }

        // 1. Call our swapchain cleanup helper first.
        cleanup_swapchain();

        // 2. Destroy synchronization primitives.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (m_frames_in_flight[i].render_finished_semaphore) vkDestroySemaphore(m_device, m_frames_in_flight[i].render_finished_semaphore, nullptr);
            if (m_frames_in_flight[i].image_available_semaphore) vkDestroySemaphore(m_device, m_frames_in_flight[i].image_available_semaphore, nullptr);
            if (m_frames_in_flight[i].in_flight_fence) vkDestroyFence(m_device, m_frames_in_flight[i].in_flight_fence], nullptr);
        }

        // 3. Destroy all remaining resources (buffers, textures, etc.)
        // This is critical. The pImpl map owns the resources.
        for (auto const &[key, val]: m_pimpl->buffers) {
            vmaDestroyBuffer(m_allocator, val.buffer, val.allocation);
        }
        // The swapchain textures were already erased, so this only handles non-swapchain textures.
        for (auto const &[key, val]: m_pimpl->textures) {
            vkDestroyImageView(m_device, val.image_view, nullptr);
            vmaDestroyImage(m_allocator, val.image, val.allocation);
        }
        // ... destroy pipelines, descriptor set layouts, etc. ...

        // 4. Destroy the VMA allocator.
        if (m_allocator) {
            vmaDestroyAllocator(m_allocator);
        }
        if (m_command_pool) {
            vkDestroyCommandPool(m_device, m_command_pool, nullptr);
        }
        // 5. Destroy the logical device.
        if (m_device) {
            vkDestroyDevice(m_device, nullptr);
        }

        // 6. Destroy the debug messenger.
        if (m_debug_messenger != VK_NULL_HANDLE) {
            // You need to load this function pointer at initialization
            // auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
            // if (func != nullptr) {
            //     func(m_instance, m_debug_messenger, nullptr);
            // }
        }

        // 7. Finally, destroy the instance.
        if (m_instance) {
            vkDestroyInstance(m_instance, nullptr);
        }
    }

    void VulkanDevice::pick_physical_device() {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
        if (device_count == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

        // Simple selection: pick the first available discrete GPU.
        // A real engine would score devices based on features and performance.
        for (const auto &device: devices) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                m_physical_device = device;
                RDE_CORE_INFO("Found discrete GPU: {}", properties.deviceName);
                return;
            }
        }
        // Fallback to the first device if no discrete GPU is found.
        if (m_physical_device == VK_NULL_HANDLE) {
            m_physical_device = devices[0];
        }
    }

    void VulkanDevice::create_logical_device() {
        // Find a queue family that supports graphics operations.
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, queue_families.data());

        for (uint32_t i = 0; i < queue_families.size(); ++i) {
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                m_graphics_queue_family_index = i;
                break;
            }
        }

        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = m_graphics_queue_family_index;
        queue_create_info.queueCount = 1;
        float queue_priority = 1.0f;
        queue_create_info.pQueuePriorities = &queue_priority;

        VkPhysicalDeviceFeatures device_features{}; // Enable features as needed

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pQueueCreateInfos = &queue_create_info;
        create_info.queueCreateInfoCount = 1;
        create_info.pEnabledFeatures = &device_features;
        // Enable device extensions like swapchain
        const char *device_extension = "VK_KHR_swapchain";
        create_info.enabledExtensionCount = 1;
        create_info.ppEnabledExtensionNames = &device_extension;


        if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }
        RDE_CORE_INFO("Vulkan Logical Device created successfully.");

        // Get a handle to the graphics queue.
        vkGetDeviceQueue(m_device, m_graphics_queue_family_index, 0, &m_graphics_queue);
    }

    void VulkanDevice::create_allocator() {
        VmaAllocatorCreateInfo allocator_info = {};
        allocator_info.vulkanApiVersion = VK_API_VERSION_1_2;
        allocator_info.physicalDevice = m_physical_device;
        allocator_info.device = m_device;
        allocator_info.instance = m_instance;

        if (vmaCreateAllocator(&allocator_info, &m_allocator) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create VMA allocator!");
        }
        RDE_CORE_INFO("VMA Allocator created successfully.");
    }

    void VulkanDevice::create_command_pool() {
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        // We want command buffers that can be reset individually
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // The pool must be tied to the queue family we will submit to
        pool_info.queueFamilyIndex = m_graphics_queue_family_index;

        if (vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
        RDE_CORE_INFO("Vulkan Command Pool created successfully.");
    }

    void VulkanDevice::find_queue_families(VkPhysicalDevice device) {
        // ... existing logic to find graphics_family ...

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, m_graphics_queue_family_index, m_surface, &present_support);
        if (!present_support) {
            // In a real engine, you'd search for a different queue family that supports both
            // graphics and present, or use separate queues.
            throw std::runtime_error("Graphics queue family does not support presentation!");
        }
    }

    void VulkanDevice::create_swapchain(const RAL::SwapchainDescription &desc) {
        // 1. Create the Surface
        // This connects Vulkan to the windowing system. It's platform-specific.
        // We use GLFW to handle this for us.
        if (!m_instance) {
            throw std::runtime_error("Vulkan instance is not initialized!");
        }
        if (!desc.native_window_handle) {
            throw std::runtime_error("Native window handle is null!");
        }
        if (!glfwVulkanSupported()) {
            throw std::runtime_error("GLFW does not support Vulkan!");
        }
        if (glfwCreateWindowSurface(m_instance, static_cast<GLFWwindow *>(desc.native_window_handle), nullptr,
                                    &m_surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }

        // Call our queue family finder again, now that we have a surface
        find_queue_families(m_physical_device);

        // 2. Query for surface capabilities, formats, and present modes
        // This is a lot of boilerplate to find the best settings.
        // ... (code to call vkGetPhysicalDeviceSurfaceCapabilitiesKHR, etc.) ...
        // For now, let's assume we found a good format (e.g., VK_FORMAT_B8G8R8A8_SRGB)
        // and a present mode (e.g., VK_PRESENT_MODE_MAILBOX_KHR for vsync-off, or FIFO for vsync-on).
        m_swapchain_image_format = VK_FORMAT_B8G8R8A8_SRGB;
        m_swapchain_extent = {desc.width, desc.height};

        // 3. Create the Swapchain
        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = m_surface;
        create_info.minImageCount = 3; // Triple buffering
        create_info.imageFormat = m_swapchain_image_format;
        create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        create_info.imageExtent = m_swapchain_extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = desc.vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain!");
        }

        // 4. Get the swapchain images
        uint32_t image_count;
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
        m_swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, m_swapchain_images.data());

        // 5. Create image views for each image
        m_swapchain_image_views.resize(image_count);
        for (size_t i = 0; i < image_count; i++) {
            // ... (code to create a VkImageView for m_swapchain_images[i]) ...
            // Store it in m_swapchain_image_views[i]

            // Wrap the VkImage/VkImageView in our VulkanTexture and store it
            VulkanTexture swapchain_texture;
            swapchain_texture.image = m_swapchain_images[i];
            swapchain_texture.image_view = m_swapchain_image_views[i];

            uint64_t new_id = m_pimpl->next_handle_id++;
            m_pimpl->textures[new_id] = swapchain_texture;

            RAL::TextureHandle ral_handle{new_id};
            m_swapchain_ral_textures.push_back(ral_handle);
        }

        // 6. Create Synchronization Primitives
        m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_available_semaphores[i]);
            vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_render_finished_semaphores[i]);
            vkCreateFence(m_device, &fence_info, nullptr, &m_in_flight_fences[i]);
        }

        m_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_command_buffers[i] = std::make_unique<VulkanCommandBuffer>(*this);
        }
    }

    void VulkanDevice::destroy_swapchain() {
        cleanup_swapchain();
    }

    VkResult VulkanDevice::acquire_next_swapchain_image(uint32_t *out_image_index) {
        auto& frame_data = m_frames_in_flight[m_current_frame];
        return vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
                                     frame_data.image_available_semaphore, VK_NULL_HANDLE, out_image_index);
    }

    RAL::CommandBuffer *VulkanDevice::begin_frame() {
        // 1. Wait for the frame we are about to reuse to be finished on the GPU.
        // This is the call that was hanging, but now it will work because end_frame() will submit work that signals this fence.
        vkWaitForFences(m_device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);

        // 2. Acquire an image from the swapchain.
        uint32_t image_index;
        VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
                                                m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE,
                                                &image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // Handle swapchain recreation... for now, signal failure.
            // cleanup_swapchain(); create_swapchain(...);
            return nullptr;
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        m_current_swapchain_image_index = image_index;

        // 3. Now that we know we're not waiting anymore, reset the fence for this frame.
        vkResetFences(m_device, 1, &m_in_flight_fences[m_current_frame]);

        // 4. Get the command buffer for the current frame and begin recording.
        VulkanCommandBuffer* cmd = m_command_buffers[m_current_frame].get();
        cmd->begin(); // Resets the VkCommandBuffer

        return cmd;
    }

    void VulkanDevice::end_frame() {
        VulkanCommandBuffer* cmd = m_command_buffers[m_current_frame].get();
        cmd->end(); // Finalizes the VkCommandBuffer

        // Submit the command buffer
        submit_command_buffers({cmd});

        // Present the result
        do_present();

        // Advance to the next frame data for the next call to begin_frame()
        m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanDevice::do_present() {
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // Wait on the semaphore that our rendering commands will signal
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &m_render_finished_semaphores[m_current_frame];

        present_info.swapchainCount = 1;
        present_info.pSwapchains = &m_swapchain;
        present_info.pImageIndices = &m_current_swapchain_image_index;

        vkQueuePresentKHR(m_graphics_queue, &present_info);

        // Advance to the next frame in flight
        m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanDevice::advance_frame() {
        // This function is called after a frame has been presented.
        // It can be used to prepare for the next frame, but in this simple case,
        // we just increment the current frame index.
        m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        m_current_swapchain_image_index = 0; // Reset for the next frame
    }

    void VulkanDevice::cleanup_swapchain() {
        if (m_device == VK_NULL_HANDLE) {
            return; // Device doesn't exist, nothing to clean
        }

        // Wait for the device to be idle to ensure no swapchain resources are in use.
        // This is a simple but heavyweight synchronization method.
        vkDeviceWaitIdle(m_device);

        // 1. Destroy Image Views
        // The VkImages are owned by the swapchain and are destroyed with it,
        // but the VkImageViews we created must be explicitly destroyed.
        for (auto image_view: m_swapchain_image_views) {
            vkDestroyImageView(m_device, image_view, nullptr);
        }
        m_swapchain_image_views.clear();

        // We also need to remove the corresponding RAL::TextureHandle entries
        // from our main texture map in the pImpl.
        for (auto ral_handle: m_swapchain_ral_textures) {
            if (ral_handle.is_valid()) {
                m_pimpl->textures.erase(ral_handle.id);
            }
        }
        m_swapchain_ral_textures.clear();


        // 2. Destroy the Swapchain itself
        if (m_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }

        // 3. Destroy the Surface
        // The surface is tied to the instance, not the logical device.
        if (m_surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
        }

        // Note: We don't destroy the synchronization primitives here because
        // they can persist across swapchain recreation. We will destroy them
        // in the main device destructor.
    }

    // --- Empty implementations for now ---
    RAL::BufferHandle VulkanDevice::create_buffer(const RAL::BufferDescription &desc) {
        // 1. Map RAL description to Vulkan/VMA structs
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = desc.size;
        buffer_info.usage = VulkanMappers::ToVkBufferUsage(desc);
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Assume not sharing between queues

        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.usage = VulkanMappers::ToVmaMemoryUsage(desc);

        // For DYNAMIC buffers, we want them to be persistently mapped for easy writing.
        if (desc.usage == RAL::ResourceUsage::DYNAMIC) {
            alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        VulkanBuffer new_vk_buffer;
        VkResult result = vmaCreateBuffer(
                m_allocator,
                &buffer_info,
                &alloc_info,
                &new_vk_buffer.buffer,
                &new_vk_buffer.allocation,
                nullptr // Optional: VmaAllocationInfo
        );

        if (result != VK_SUCCESS) {
            // RDE_CORE_ERROR("Failed to create buffer!");
            return {}; // Return invalid handle
        }

        // 2. Store the new VulkanBuffer and generate a RAL handle
        uint64_t new_id = m_pimpl->next_handle_id++;
        m_pimpl->buffers[new_id] = new_vk_buffer;

        RAL::BufferHandle ral_handle;
        ral_handle.id = new_id;

        // Optional: Set a debug name for tools like RenderDoc
        // set_debug_name(new_vk_buffer.buffer, desc.name);

        return ral_handle;
    }


    void VulkanDevice::destroy_buffer(RAL::BufferHandle handle) {
        if (!handle.is_valid()) return;

        auto it = m_pimpl->buffers.find(handle.id);
        if (it != m_pimpl->buffers.end()) {
            VulkanBuffer &vk_buffer = it->second;

            // Use VMA to destroy both the buffer and its memory allocation.
            vmaDestroyBuffer(m_allocator, vk_buffer.buffer, vk_buffer.allocation);

            // Remove from our map.
            m_pimpl->buffers.erase(it);
        }
    }

    RAL::TextureHandle VulkanDevice::create_texture(const RAL::TextureDescription &desc) {
        // 1. Map RAL description to Vulkan/VMA structs
        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        // For now, assume 2D textures. A full implementation would switch on a type enum.
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = desc.width;
        image_info.extent.height = desc.height;
        image_info.extent.depth = 1;
        image_info.mipLevels = desc.mip_levels;
        image_info.arrayLayers = 1;
        image_info.format = VulkanMappers::ToVkFormat(desc.format);
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL; // Always use optimal tiling for performance
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Start in undefined state
        image_info.usage = VulkanMappers::ToVkImageUsage(desc);
        image_info.samples = VK_SAMPLE_COUNT_1_BIT; // No multisampling for now
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.usage = VulkanMappers::ToVmaMemoryUsage(desc);

        VulkanTexture new_vk_texture;
        VkResult result = vmaCreateImage(
                m_allocator,
                &image_info,
                &alloc_info,
                &new_vk_texture.image,
                &new_vk_texture.allocation,
                nullptr
        );

        if (result != VK_SUCCESS) {
            // RDE_CORE_ERROR("Failed to create texture image!");
            return {}; // Return invalid handle
        }

        // 2. Create the Image View
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = new_vk_texture.image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // Match imageType
        view_info.format = image_info.format;
        view_info.subresourceRange.aspectMask = (image_info.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = desc.mip_levels;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device, &view_info, nullptr, &new_vk_texture.image_view) != VK_SUCCESS) {
            // RDE_CORE_ERROR("Failed to create texture image view!");
            // Cleanup the image we just created
            vmaDestroyImage(m_allocator, new_vk_texture.image, new_vk_texture.allocation);
            return {};
        }

        // 3. Store the new VulkanTexture and generate a RAL handle
        uint64_t new_id = m_pimpl->next_handle_id++;
        m_pimpl->textures[new_id] = new_vk_texture;

        RAL::TextureHandle ral_handle;
        ral_handle.id = new_id;

        // Optional: Set debug names for RenderDoc
        // set_debug_name(new_vk_texture.image, desc.name);
        // set_debug_name(new_vk_texture.image_view, desc.name + "_view");

        return ral_handle;
    }

    void VulkanDevice::destroy_texture(RAL::TextureHandle handle) {
        if (!handle.is_valid()) return;

        auto it = m_pimpl->textures.find(handle.id);
        if (it != m_pimpl->textures.end()) {
            VulkanTexture &vk_texture = it->second;

            // Destroy the view first, then the image
            vkDestroyImageView(m_device, vk_texture.image_view, nullptr);
            vmaDestroyImage(m_allocator, vk_texture.image, vk_texture.allocation);

            m_pimpl->textures.erase(it);
        }
    }

    RAL::PipelineHandle VulkanDevice::create_pipeline(const RAL::PipelineDescription &desc) {
        // This is a placeholder. A full implementation would create a VkPipeline
        // and store it in a map similar to buffers and textures.
        // For now, we return an invalid handle.
        RAL::PipelineHandle handle;
        handle.id = 0; // Invalid ID
        RDE_CORE_INFO("TODO: Create Vulkan pipeline for description");
        return handle;
    }

    void VulkanDevice::destroy_pipeline(RAL::PipelineHandle handle) {
        RDE_CORE_INFO("TODO: Destroy Vulkan pipeline with ID: {}", handle.id);
        if (!handle.is_valid()) return;
    }

    RAL::DescriptorSetLayoutHandle VulkanDevice::create_descriptor_set_layout(const RAL::DescriptorSetLayoutDescription& desc) {
        RDE_CORE_INFO("TODO: Create Vulkan descriptor set layout for description");
        return {};
    }

    void VulkanDevice::destroy_descriptor_set_layout(RAL::DescriptorSetLayoutHandle handle) {
        RDE_CORE_INFO("TODO: Destroy Vulkan descriptor set layout with ID: {}", handle.id);
    }

    std::vector<RAL::DescriptorSetHandle> VulkanDevice::allocate_descriptor_sets(RAL::DescriptorSetLayoutHandle layout, uint32_t count) {
        RDE_CORE_INFO("TODO: Allocate {} Vulkan descriptor sets for layout ID: {}", count, layout.id);
        return {};
    }

    void VulkanDevice::update_descriptor_sets(const std::vector<RAL::WriteDescriptorSet>& writes) {
        RDE_CORE_INFO("TODO: Update Vulkan descriptor sets with {} writes", writes.size());
    }

    std::unique_ptr<RAL::CommandBuffer> VulkanDevice::create_command_buffer() {
        // This one is important for the next step. We can implement it properly now.
        // It requires a VulkanCommandBuffer class.
        return std::make_unique<VulkanCommandBuffer>(*this);
    }

    void VulkanDevice::submit_command_buffers(const std::vector<RAL::CommandBuffer*>& command_buffers) {
        if (command_buffers.empty()) return;

        std::vector<VkCommandBuffer> vk_command_buffers;
        vk_command_buffers.reserve(command_buffers.size());
        for (const auto& cmd : command_buffers) {
            // We must cast from the abstract RAL interface to our concrete Vulkan implementation.
            vk_command_buffers.push_back(static_cast<VulkanCommandBuffer*>(cmd)->get_native_handle());
        }

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // Wait on the image_available semaphore before the color attachment output stage.
        VkSemaphore wait_semaphores[] = { m_image_available_semaphores[m_current_frame] };
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;

        submit_info.commandBufferCount = static_cast<uint32_t>(vk_command_buffers.size());
        submit_info.pCommandBuffers = vk_command_buffers.data();

        // Signal the render_finished semaphore when commands are done.
        VkSemaphore signal_semaphores[] = { m_render_finished_semaphores[m_current_frame] };
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;

        // The in_flight_fence will be signaled by the GPU when this submission completes.
        // This is what `begin_frame` will wait on.
        if (vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }
    }

    void VulkanDevice::wait_idle() {
        // This one is easy and correct to implement fully now.
        vkDeviceWaitIdle(m_device);
    }
    // ... all others
}