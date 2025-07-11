#include "VulkanContext.h"
#include "VulkanCommon.h"
#include "core/Log.h"

#include <vector>
#include <stdexcept>
#include <set>

namespace RDE {
    namespace { // Use an anonymous namespace for file-local helpers
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
    }

    VulkanContext::VulkanContext(GLFWwindow *window) {
        create_instance();
        setup_debug_messenger();
        create_surface(window);
        pick_physical_device();
        create_logical_device();
        create_vma_allocator();
        RDE_CORE_INFO("Vulkan Context Initialized successfully.");
    }

    VulkanContext::~VulkanContext() {
        if (m_VmaAllocator != VK_NULL_HANDLE) {
            vmaDestroyAllocator(m_VmaAllocator);
        }
        if (m_LogicalDevice != VK_NULL_HANDLE) {
            vkDestroyDevice(m_LogicalDevice, nullptr);
        }
        if (m_DebugMessenger != VK_NULL_HANDLE) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance,
                                                                                   "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(m_Instance, m_DebugMessenger, nullptr);
            } else {
                RDE_CORE_ERROR("Failed to load vkDestroyDebugUtilsMessengerEXT function pointer.");
            }
        }
        if (m_Surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        }
        if (m_Instance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_Instance, nullptr);
        }
    }

    // Implementations of the private methods would go here...

    void VulkanContext::create_instance() {
        // MOVED FROM: VulkanDevice constructor block 1
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "RDEngine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "RDEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = RDE_USED_VK_VERSION; // Using 1.3 for Dynamic Rendering

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
    }

    void VulkanContext::setup_debug_messenger() {
        // MOVED FROM: VulkanDevice constructor block 2
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType =
                VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;

        //Why do we use vkGetInstanceProcAddr here? cant we just use vkCreateDebugUtilsMessengerEXT directly?
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance,
                                                                               "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            VK_CHECK(func(m_Instance, &createInfo, nullptr, &m_DebugMessenger));
        } else {
            RDE_CORE_ERROR("Failed to load vkCreateDebugUtilsMessengerEXT function pointer.");
        }
    }

    void VulkanContext::create_surface(GLFWwindow *window) {
        // MOVED FROM: VulkanDevice constructor block 3
        VK_CHECK(glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface));
    }

    void VulkanContext::pick_physical_device() {
        // MOVED FROM: VulkanDevice constructor block 4
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        // A proper implementation would score devices, but we'll take the first for now.
        m_PhysicalDevice = devices[0]; // TODO: Implement proper device scoring

        // Find queue families
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

        bool foundGraphics = false;
        bool foundPresent = false;
        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                m_GraphicsQueueFamilyIndex = i;
                foundGraphics = true;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &presentSupport);
            if (presentSupport) {
                m_PresentQueueFamilyIndex = i;
                foundPresent = true;
            }

            if (foundGraphics && foundPresent) {
                break; // Found all necessary queues
            }
        }
        if (!foundGraphics || !foundPresent) {
            throw std::runtime_error("Failed to find suitable queue families!");
        }

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);
        RDE_CORE_INFO("Selected GPU: {}", properties.deviceName);
    }

    void VulkanContext::create_logical_device() {
        // MOVED FROM: VulkanDevice constructor block 5
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {m_GraphicsQueueFamilyIndex, m_PresentQueueFamilyIndex};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily: uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // --- Features we need ---
        VkPhysicalDeviceFeatures supportedFeatures{};
        vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &supportedFeatures);

        VkPhysicalDeviceFeatures enabledFeatures{};
        if (supportedFeatures.samplerAnisotropy) {
            enabledFeatures.samplerAnisotropy = VK_TRUE;
        } else {
            RDE_CORE_WARN("Sampler Anisotropy is not supported; textures may look blurry at angles.");
        }

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
        dynamicRenderingFeature.dynamicRendering = VK_TRUE;

        // --- Device Creation ---
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &dynamicRenderingFeature; // Chain dynamic rendering features
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &enabledFeatures;

        const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        VK_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice));

        // Retrieve queue handles
        vkGetDeviceQueue(m_LogicalDevice, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_LogicalDevice, m_PresentQueueFamilyIndex, 0, &m_PresentQueue);
    }

    void VulkanContext::create_vma_allocator() {
        // MOVED FROM: VulkanDevice constructor VMA block
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.vulkanApiVersion = RDE_USED_VK_VERSION;
        allocatorInfo.physicalDevice = m_PhysicalDevice;
        allocatorInfo.device = m_LogicalDevice;
        allocatorInfo.instance = m_Instance;

        VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_VmaAllocator));
    }
}