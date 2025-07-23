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
#include <utility>
#include <vector>
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

    VulkanDevice::VulkanDevice(std::shared_ptr<VulkanContext> context, std::shared_ptr<VulkanSwapchain> swapchain)
            : m_Context(std::move(context)), m_Swapchain(std::move(swapchain)) {
        auto logicalDevice = m_Context->get_logical_device();

        // === 1. Create Command Pool ===
        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = m_Context->get_graphics_queue_family();
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            VK_CHECK(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &m_CommandPool));
        }

        // === 2. Create Upload Context ===
        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = m_Context->get_graphics_queue_family();
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            VK_CHECK(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &m_UploadCommandPool));

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = m_UploadCommandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            VK_CHECK(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &m_UploadCommandBuffer));

            VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            VK_CHECK(vkCreateFence(logicalDevice, &fenceInfo, nullptr, &m_UploadFence));
        }
        // === 3. Create Global Descriptor Pool ===
        {
            std::vector<VkDescriptorPoolSize> poolSizes = {
                    {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000}
            };
            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            poolInfo.maxSets = 1000 * poolSizes.size();
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            VK_CHECK(vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &m_DescriptorPool));
        }
        // === 4. Create Frame Sync Objects & Command Buffers ===
        {
            m_FrameDeletionQueues.resize(FRAMES_IN_FLIGHT);
            m_ImageAvailableSemaphores.resize(FRAMES_IN_FLIGHT);
            m_RenderFinishedSemaphores.resize(FRAMES_IN_FLIGHT);
            m_InFlightFences.resize(FRAMES_IN_FLIGHT);
            m_FrameCommandBuffers.resize(FRAMES_IN_FLIGHT);

            VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = m_CommandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
                VK_CHECK(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
                VK_CHECK(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
                VK_CHECK(vkCreateFence(logicalDevice, &fenceInfo, nullptr, &m_InFlightFences[i]));

                VkCommandBuffer vk_cmd;
                VK_CHECK(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &vk_cmd));
                m_FrameCommandBuffers[i] = std::make_unique<VulkanCommandBuffer>(vk_cmd, this);
            }
        }
        RDE_CORE_INFO("Vulkan Device Initialized successfully.");
    }

    VulkanDevice::~VulkanDevice() {
        wait_idle();
        auto logicalDevice = m_Context->get_logical_device();

        // Destroy only what WE own. Context and Swapchain manage their own lifetimes.
        for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {
            m_FrameDeletionQueues[i].flush();
            vkDestroySemaphore(logicalDevice, m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(logicalDevice, m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(logicalDevice, m_InFlightFences[i], nullptr);
        }

        vkDestroyDescriptorPool(logicalDevice, m_DescriptorPool, nullptr);
        vkDestroyCommandPool(logicalDevice, m_CommandPool, nullptr);

        vkDestroyFence(logicalDevice, m_UploadFence, nullptr);
        vkDestroyCommandPool(logicalDevice, m_UploadCommandPool, nullptr);
    }

    RAL::CommandBuffer *VulkanDevice::begin_frame() {
        auto logicalDevice = m_Context->get_logical_device();

        VK_CHECK(vkWaitForFences(logicalDevice, 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX));

        VkResult result = m_Swapchain->acquire_next_image(m_ImageAvailableSemaphores[m_CurrentFrameIndex]);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            m_Swapchain->recreate();
            return nullptr; // Indicate to the caller that the frame should be skipped
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        // We can now safely reset the fence as we are about to use its frame resources
        VK_CHECK(vkResetFences(logicalDevice, 1, &m_InFlightFences[m_CurrentFrameIndex]));

        m_FrameDeletionQueues[m_CurrentFrameIndex].flush();

        VulkanCommandBuffer *cmd = m_FrameCommandBuffers[m_CurrentFrameIndex].get();
        VK_CHECK(vkResetCommandBuffer(cmd->get_handle(), 0));
        cmd->begin();

        cmd->transition_image_layout(m_Swapchain->get_current_image(), VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        return cmd;
    }

    void VulkanDevice::end_frame(const std::vector<RAL::CommandBuffer *> &command_buffers) {
        VulkanCommandBuffer *main_cmd = dynamic_cast<VulkanCommandBuffer *>(command_buffers[0]);
        // Assuming first is main

        main_cmd->transition_image_layout(m_Swapchain->get_current_image(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        main_cmd->end();

        std::vector<VkCommandBuffer> vkCommandBuffers;
        vkCommandBuffers.reserve(command_buffers.size());
        for (const auto &cmd: command_buffers) {
            vkCommandBuffers.push_back(dynamic_cast<VulkanCommandBuffer *>(cmd)->get_handle());
        }

        submit_internal(vkCommandBuffers);

        VkResult result = m_Swapchain->present(m_RenderFinishedSemaphores[m_CurrentFrameIndex],
                                               m_Context->get_present_queue());
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            m_Swapchain->recreate();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % FRAMES_IN_FLIGHT;
    }

    void VulkanDevice::submit_internal(const std::vector<VkCommandBuffer> &vkCommandBuffers) {
        // CHANGED: Now gets queue from context
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrameIndex]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = static_cast<uint32_t>(vkCommandBuffers.size());
        submitInfo.pCommandBuffers = vkCommandBuffers.data();

        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrameIndex]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK(vkQueueSubmit(m_Context->get_graphics_queue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrameIndex]));
    }


    void *VulkanDevice::map_buffer(RAL::BufferHandle handle) {
        if (!m_resources_db.is_valid(handle)) {
            return nullptr;
        }
        auto &buffer = m_resources_db.get<VulkanBuffer>(handle);
        VmaAllocator allocator = m_Context->get_vma_allocator();
        void *mappedData = nullptr;
        VkResult result = vmaMapMemory(allocator, buffer.allocation, &mappedData);

        if (result != VK_SUCCESS) {
            RDE_CORE_ERROR("VMA: Failed to map buffer! Error: {}", ToString(result));
            return nullptr;
        }

        return mappedData;
    }

    void VulkanDevice::unmap_buffer(RAL::BufferHandle handle) {
        if (!m_resources_db.is_valid(handle)) {
            return;
        }
        auto &buffer = m_resources_db.get<VulkanBuffer>(handle);
        VmaAllocator allocator = m_Context->get_vma_allocator();
        vmaUnmapMemory(allocator, buffer.allocation);
    }

    void
    VulkanDevice::update_buffer_data(RAL::BufferHandle target_handle, const void *data, size_t size, size_t offset) {
        if (!m_resources_db.is_valid(target_handle)) {
            RDE_CORE_ERROR("Attempted to update an invalid buffer handle: {}", entt::to_integral(target_handle.index));
            return;
        }

        auto &target_buffer_info = m_resources_db.get<VulkanBuffer>(target_handle);
        // --- INTELLIGENT PATH SELECTION ---
        // Check how the buffer was created to decide on the update strategy.

        if (target_buffer_info.memoryUsage == RAL::MemoryUsage::HostVisible /*||
            target_buffer_info.memoryUsage == RAL::MemoryUsage::HostVisibleCoherent*/) {

            // --- PATH 1: SIMPLE MAP/MEMCPY (Using your existing functions) ---
            // This is for CPU-visible buffers. It's simple and direct.
            RDE_CORE_TRACE("Updating HostVisible buffer via map/memcpy.");

            void *mapped_data = this->map_buffer(target_handle);
            if (mapped_data) {
                // memcpy to the pointer at the correct offset
                memcpy(static_cast<uint8_t *>(mapped_data) + offset, data, size);
                this->unmap_buffer(target_handle);
            } else {
                RDE_CORE_ERROR("Failed to map buffer {} for update.", entt::to_integral(target_handle.index));
            }

        } else { // Assumes RAL::MemoryUsage::DeviceLocal

            // --- PATH 2: OPTIMAL STAGING BUFFER TRANSFER ---
            // This is for GPU-only memory. It's more complex but higher performance.
            RDE_CORE_TRACE("Updating DeviceLocal buffer via staging transfer.");

            auto vma_allocator = m_Context->get_vma_allocator();

            // Create and fill a temporary staging buffer
            VulkanBuffer staging_buffer;

            VkBufferCreateInfo staging_buffer_ci{};
            staging_buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            staging_buffer_ci.size = size;
            staging_buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            VmaAllocationCreateInfo staging_alloc_ci{};
            staging_alloc_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            staging_alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

            VK_CHECK(vmaCreateBuffer(vma_allocator, &staging_buffer_ci, &staging_alloc_ci, &staging_buffer.handle,
                                     &staging_buffer.allocation, nullptr));

            void *mapped_data;
            vmaMapMemory(vma_allocator, staging_buffer.allocation, &mapped_data);
            memcpy(mapped_data, data, size);
            vmaUnmapMemory(vma_allocator, staging_buffer.allocation);

            // Issue the copy command
            immediate_submit([&](VkCommandBuffer cmd) {
                VkBufferCopy copy_region{};
                copy_region.srcOffset = 0;
                copy_region.dstOffset = offset;
                copy_region.size = size;

                vkCmdCopyBuffer(cmd, staging_buffer.handle, target_buffer_info.handle, 1, &copy_region);
            });

            // Clean up
            vmaDestroyBuffer(vma_allocator, staging_buffer.handle, staging_buffer.allocation);
        }
    }

    void VulkanDevice::copy_buffer(RAL::BufferHandle source_handle, RAL::BufferHandle target_handle, size_t size,
                                   size_t source_offset, size_t target_offset) {
        if (!m_resources_db.is_valid(source_handle) || !m_resources_db.is_valid(target_handle)) {
            RDE_CORE_ERROR("Attempted to copy with an invalid buffer handle.");
            return;
        }

        const auto &source_buffer = m_resources_db.get<VulkanBuffer>(source_handle);
        const auto &destination_buffer = m_resources_db.get<VulkanBuffer>(target_handle);

        // If size is -1, copy the entire source buffer (respecting offsets).
        size_t copy_size = size;
        if (copy_size == (size_t) -1) {
            copy_size = source_buffer.size - source_offset;
        }

        // Safety check
        if (copy_size > (destination_buffer.size - target_offset)) {
            RDE_CORE_ERROR("GPU buffer copy would overflow destination buffer.");
            return;
        }

        immediate_submit([&](VkCommandBuffer cmd) {
            VkBufferCopy copy_region{};
            copy_region.srcOffset = source_offset;
            copy_region.dstOffset = target_offset;
            copy_region.size = copy_size;
            vkCmdCopyBuffer(cmd, source_buffer.handle, destination_buffer.handle, 1, &copy_region);
        });
    }

    void VulkanDevice::immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function) {
        // Get handles from the context
        VkDevice logicalDevice = m_Context->get_logical_device();
        VkQueue graphicsQueue = m_Context->get_graphics_queue();

        // The rest of the logic is the same, just using the local variables above
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(m_UploadCommandBuffer, &beginInfo));

        function(m_UploadCommandBuffer);

        VK_CHECK(vkEndCommandBuffer(m_UploadCommandBuffer));

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_UploadCommandBuffer;
        VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_UploadFence));

        VK_CHECK(vkWaitForFences(logicalDevice, 1, &m_UploadFence, VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(logicalDevice, 1, &m_UploadFence));

        VK_CHECK(vkResetCommandPool(logicalDevice, m_UploadCommandPool, 0));
    }

    void VulkanDevice::copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        immediate_submit([&](VkCommandBuffer cmd) {
            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
        });
    }

    VkShaderModule VulkanDevice::create_shader_module(const std::vector<char> &code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        VkDevice logicalDevice = m_Context->get_logical_device();
        VK_CHECK(vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }

    DeletionQueue &VulkanDevice::get_current_frame_deletion_queue() {
        return m_FrameDeletionQueues[m_CurrentFrameIndex];
    }

    // Implement the public interface function
    RAL::ShaderHandle VulkanDevice::create_shader(const RAL::ShaderDescription &desc) {
        auto shaderCode = FileIO::ReadFile(desc.filePath);
        return create_shader_module(shaderCode, desc.stage);
    }

    RAL::ShaderHandle VulkanDevice::create_shader_module(const std::vector<char> &bytecode, RAL::ShaderStage stage) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = bytecode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(bytecode.data());

        VkShaderModule vk_shader_module;
        VkDevice logicalDevice = m_Context->get_logical_device();
        VK_CHECK(vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &vk_shader_module));

        // 2. Create the handle/entity in the database
        auto handle = RAL::ShaderHandle{m_resources_db.create()};

        // 3. Emplace the components
        m_resources_db.emplace<VulkanShader>(handle, vk_shader_module); // Your Vulkan-specific struct
        m_resources_db.emplace<RAL::ShaderStage>(handle, stage); // The metadata!
        return handle;
    }

    void VulkanDevice::destroy_shader(RAL::ShaderHandle handle) {
        if (!m_resources_db.is_valid(handle)) return;

        const auto &vk_shader = m_resources_db.get<VulkanShader>(handle);
        VkDevice logicalDevice = m_Context->get_logical_device();

        get_current_frame_deletion_queue().push([=]() {
            vkDestroyShaderModule(logicalDevice, vk_shader.module, nullptr);
        });

        m_resources_db.destroy(handle);
    }

    RAL::PipelineHandle VulkanDevice::create_pipeline(const RAL::PipelineDescription &desc) {
        VulkanPipeline newPipeline;
        auto logicalDevice = m_Context->get_logical_device();

        // --- 1. Shader Stages ---
        const auto &vs = m_resources_db.get<VulkanShader>(desc.vertexShader);
        const auto &fs = m_resources_db.get<VulkanShader>(desc.fragmentShader);

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
            attributes.push_back({
                                         .location = a.location, .binding = a.binding, .format = ToVulkanFormat(
                            a.format),
                                         .offset = a.offset
                                 });
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
        rasterizer.polygonMode = ToVulkanPolygonMode(
                desc.rasterizationState.polygonMode); // Map from desc.rasterizationState later
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = ToVulkanCullMode(
                desc.rasterizationState.cullMode); // Map from desc.rasterizationState later
        rasterizer.frontFace = ToVulkanFrontFace(
                desc.rasterizationState.frontFace); // Map from desc.rasterizationState later
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = desc.colorBlendState.attachment.blendEnable; // VK_TRUE for ImGui

        const auto &ralBlend = desc.colorBlendState.attachment;
        colorBlendAttachment.srcColorBlendFactor = ToVulkanBlendFactor(ralBlend.srcColorBlendFactor);
        colorBlendAttachment.dstColorBlendFactor = ToVulkanBlendFactor(ralBlend.dstColorBlendFactor);
        colorBlendAttachment.colorBlendOp = ToVulkanBlendOp(ralBlend.colorBlendOp);
        colorBlendAttachment.srcAlphaBlendFactor = ToVulkanBlendFactor(ralBlend.srcAlphaBlendFactor);
        colorBlendAttachment.dstAlphaBlendFactor = ToVulkanBlendFactor(ralBlend.dstAlphaBlendFactor);
        colorBlendAttachment.alphaBlendOp = ToVulkanBlendOp(ralBlend.alphaBlendOp);

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        // --- 4. Pipeline Layout ---
        std::vector<VkDescriptorSetLayout> vkSetLayouts;
        for (const auto &layoutHandle: desc.descriptorSetLayouts) {
            const auto &vk_layout = m_resources_db.get<VulkanDescriptorSetLayout>(layoutHandle);
            vkSetLayouts.push_back(vk_layout.handle);
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
        VK_CHECK(vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &newPipeline.layout));

        // --- Dynamic Rendering ---
        VkFormat swapchainImageFormat = m_Swapchain->get_image_format();

        VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchainImageFormat;
        // We are not using a depth buffer yet
        pipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

        // --- 5. Create the Graphics Pipeline ---
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.pNext = &pipelineRenderingCreateInfo;
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
        pipelineInfo.subpass = 0;

        VK_CHECK(
                vkCreateGraphicsPipelines(logicalDevice,
                                          VK_NULL_HANDLE,
                                          1,
                                          &pipelineInfo,
                                          nullptr,
                                          &newPipeline.handle)
        );

        auto handle = RAL::PipelineHandle{m_resources_db.create()};
        m_resources_db.emplace<VulkanPipeline>(handle, newPipeline);
        m_resources_db.emplace<RAL::PipelineDescription>(handle, desc); // Store the recipe!

        return handle;
    }

    void VulkanDevice::destroy_pipeline(RAL::PipelineHandle handle) {
        if (!m_resources_db.is_valid(handle)) return;

        const auto &vk_pipeline = m_resources_db.get<VulkanPipeline>(handle);
        VkDevice logicalDevice = m_Context->get_logical_device();

        get_current_frame_deletion_queue().push([=]() {
            vkDestroyPipeline(logicalDevice, vk_pipeline.handle, nullptr);
            vkDestroyPipelineLayout(logicalDevice, vk_pipeline.layout, nullptr);
        });

        m_resources_db.destroy(handle);
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
        VkDevice logicalDevice = m_Context->get_logical_device();
        VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &layout.handle));

        auto handle = RAL::DescriptorSetLayoutHandle{m_resources_db.create()};
        m_resources_db.emplace<VulkanDescriptorSetLayout>(handle, layout);
        m_resources_db.emplace<RAL::DescriptorSetLayoutDescription>(handle, desc); // Store the recipe!

        return handle;
    }

    void VulkanDevice::destroy_descriptor_set_layout(RAL::DescriptorSetLayoutHandle handle) {

        if (!m_resources_db.is_valid(handle)) return;

        const auto &vk_descriptor_set_layout = m_resources_db.get<VulkanDescriptorSetLayout>(handle);
        VkDevice logicalDevice = m_Context->get_logical_device();

        get_current_frame_deletion_queue().push([=]() {
            vkDestroyDescriptorSetLayout(logicalDevice, vk_descriptor_set_layout.handle, nullptr);
        });

        m_resources_db.destroy(handle);
    }

    RAL::DescriptorSetHandle VulkanDevice::create_descriptor_set(const RAL::DescriptorSetDescription &desc) {
        if (!m_resources_db.is_valid(desc.layout)) {
            RDE_CORE_ERROR("Attempted to create descriptor set with an invalid layout handle!");
            return RAL::DescriptorSetHandle::INVALID();
        }
        const auto &vk_layout = m_resources_db.get<VulkanDescriptorSetLayout>(desc.layout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &vk_layout.handle;

        VkDescriptorSet new_vk_set;
        VkDevice logicalDevice = m_Context->get_logical_device();
        VK_CHECK(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &new_vk_set));

        std::vector<VkWriteDescriptorSet> descriptorWrites;
        // These must live until vkUpdateDescriptorSets is called, so they are declared outside the loop.
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkDescriptorImageInfo> imageInfos;

        for (const auto &ralWrite: desc.writes) {
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = new_vk_set;
            write.dstBinding = ralWrite.binding;
            write.dstArrayElement = 0;
            write.descriptorType = ToVulkanDescriptorType(ralWrite.type);
            write.descriptorCount = 1;

            switch (ralWrite.type) {
                case RAL::DescriptorType::UniformBuffer: {
                    // Get the concrete Vulkan buffer component from the database
                    if (!m_resources_db.is_valid(ralWrite.buffer)) continue;
                    const auto &vk_buffer = m_resources_db.get<VulkanBuffer>(ralWrite.buffer);
                    bufferInfos.push_back({vk_buffer.handle, 0, VK_WHOLE_SIZE});
                    write.pBufferInfo = &bufferInfos.back();
                    break;
                }
                case RAL::DescriptorType::StorageBuffer: {
                    // Get the concrete Vulkan buffer component from the database
                    if (!m_resources_db.is_valid(ralWrite.buffer)) continue;
                    const auto &vk_buffer = m_resources_db.get<VulkanBuffer>(ralWrite.buffer);
                    bufferInfos.push_back({vk_buffer.handle, 0, VK_WHOLE_SIZE});
                    write.pBufferInfo = &bufferInfos.back();
                    break;
                }
                case RAL::DescriptorType::SampledImage: {
                    // Get the concrete Vulkan texture component
                    if (!m_resources_db.is_valid(ralWrite.texture)) continue;
                    const auto &vk_texture = m_resources_db.get<VulkanTexture>(ralWrite.texture);
                    imageInfos.push_back(
                            {VK_NULL_HANDLE, vk_texture.image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
                    write.pImageInfo = &imageInfos.back();
                    break;
                }
                case RAL::DescriptorType::StorageImage: {
                    // Get the concrete Vulkan texture component
                    if (!m_resources_db.is_valid(ralWrite.texture)) continue;
                    const auto &vk_texture = m_resources_db.get<VulkanTexture>(ralWrite.texture);
                    imageInfos.push_back(
                            {VK_NULL_HANDLE, vk_texture.image_view, VK_IMAGE_LAYOUT_GENERAL});
                    write.pImageInfo = &imageInfos.back();
                    break;
                }
                case RAL::DescriptorType::Sampler : {
                    // Get the concrete Vulkan sampler component
                    if (!m_resources_db.is_valid(ralWrite.sampler)) continue;
                    const auto &vk_sampler = m_resources_db.get<VulkanSampler>(ralWrite.sampler);
                    imageInfos.push_back({vk_sampler.handle, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED});
                    write.pImageInfo = &imageInfos.back();
                    break;
                }
                case RAL::DescriptorType::CombinedImageSampler: {
                    // Get the concrete Vulkan texture and sampler components
                    if (!m_resources_db.is_valid(ralWrite.texture) ||
                        !m_resources_db.is_valid(ralWrite.sampler))
                        continue;
                    const auto &vk_texture = m_resources_db.get<VulkanTexture>(ralWrite.texture);
                    const auto &vk_sampler = m_resources_db.get<VulkanSampler>(ralWrite.sampler);
                    imageInfos.push_back(
                            {vk_sampler.handle, vk_texture.image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
                    write.pImageInfo = &imageInfos.back();
                    break;
                }
            }
            descriptorWrites.push_back(write);
        }

        vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
                               0, nullptr);

        // --- 4. Create the handle and emplace the components into the database ---
        auto handle = RAL::DescriptorSetHandle{m_resources_db.create()};
        m_resources_db.emplace<VulkanDescriptorSet>(handle, new_vk_set);
        m_resources_db.emplace<RAL::DescriptorSetDescription>(handle, desc); // Store the recipe

        return handle;
    }

    void VulkanDevice::destroy_descriptor_set(RAL::DescriptorSetHandle handle) {
        if (!m_resources_db.is_valid(handle)) return;

        // Get the Vulkan component so we have the native handle to free
        const auto &vk_set_comp = m_resources_db.get<VulkanDescriptorSet>(handle);

        VkDevice logicalDevice = m_Context->get_logical_device();
        VkDescriptorPool descriptorPool = m_DescriptorPool; // The pool it came from

        // Queue the "free" operation. This returns the set's memory to the pool.
        get_current_frame_deletion_queue().push([=]() {
            // Important: We capture vk_set_comp.handle, not the component itself,
            // as the component will be destroyed by m_resources_db.destroy() before this lambda runs.
            VkDescriptorSet set_to_free = vk_set_comp.handle;
            vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &set_to_free);
        });

        // Destroy the entity in our database, freeing the RAL handle for reuse.
        m_resources_db.destroy(handle);
    }

    RAL::SamplerHandle VulkanDevice::create_sampler(const RAL::SamplerDescription &desc) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = ToVulkanFilter(desc.magFilter);
        samplerInfo.minFilter = ToVulkanFilter(desc.minFilter);
        samplerInfo.addressModeU = ToVulkanAddressMode(desc.addressModeU);
        samplerInfo.addressModeV = ToVulkanAddressMode(desc.addressModeV);
        samplerInfo.addressModeW = ToVulkanAddressMode(desc.addressModeW);
        {
            const auto &properties = m_Context->get_physical_device_properties();
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(m_Context->get_physical_device(), &features);

            if (features.samplerAnisotropy) {
                samplerInfo.anisotropyEnable = VK_TRUE;
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            } else {
                samplerInfo.anisotropyEnable = VK_FALSE;
                samplerInfo.maxAnisotropy = 1.0f; // Must be 1.0 if anisotropy is disabled
            }
        }

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

        VulkanSampler sampler;
        VkDevice logicalDevice = m_Context->get_logical_device();
        VK_CHECK(vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &sampler.handle));

        auto handle = RAL::SamplerHandle{m_resources_db.create()};
        m_resources_db.emplace<VulkanSampler>(handle, sampler);
        m_resources_db.emplace<RAL::SamplerDescription>(handle, desc); // Store the recipe!

        return handle;
    }

    void VulkanDevice::destroy_sampler(RAL::SamplerHandle handle) {
        if (!m_resources_db.is_valid(handle)) return;

        const auto &vk_sampler = m_resources_db.get<VulkanSampler>(handle);
        VkDevice logicalDevice = m_Context->get_logical_device();

        get_current_frame_deletion_queue().push([=]() {
            vkDestroySampler(logicalDevice, vk_sampler.handle, nullptr);
        });

        m_resources_db.destroy(handle);
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
        VmaAllocator allocator = m_Context->get_vma_allocator();
        VulkanBuffer newBuffer;
        VK_CHECK(
                vmaCreateBuffer(allocator,
                                &bufferInfo,
                                &allocInfo,
                                &newBuffer.handle,
                                &newBuffer.allocation,
                                nullptr)
        );

        if (desc.initialData) {
            if (desc.memoryUsage == RAL::MemoryUsage::HostVisible) {
                void *mappedData;
                vmaMapMemory(allocator, newBuffer.allocation, &mappedData);
                memcpy(mappedData, desc.initialData, desc.size);
                vmaUnmapMemory(allocator, newBuffer.allocation);
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
                        vmaCreateBuffer(allocator,
                                        &stagingBufferInfo,
                                        &stagingAllocInfo,
                                        &stagingBuffer.handle,
                                        &stagingBuffer.allocation,
                                        nullptr)
                );

                void *mappedData;
                vmaMapMemory(allocator, stagingBuffer.allocation, &mappedData);
                memcpy(mappedData, desc.initialData, desc.size);
                vmaUnmapMemory(allocator, stagingBuffer.allocation);

                copy_buffer(stagingBuffer.handle, newBuffer.handle, desc.size);

                vmaDestroyBuffer(allocator, stagingBuffer.handle, stagingBuffer.allocation);
            }
        }

        auto handle = RAL::BufferHandle{this->m_resources_db.create()};

        this->m_resources_db.emplace<RAL::BufferDescription>(handle, desc);
        this->m_resources_db.emplace<VulkanBuffer>(handle, newBuffer);

        return handle;
    }

    void VulkanDevice::destroy_buffer(RAL::BufferHandle handle) {
        // 1. Check for validity using the database
        if (!m_resources_db.is_valid(handle)) {
            return;
        }

        // 2. Get the Vulkan-specific component to destroy the GPU object
        const auto &vk_buffer = m_resources_db.get<VulkanBuffer>(handle);
        VmaAllocator allocator = m_Context->get_vma_allocator();

        // 3. Queue the GPU destruction
        get_current_frame_deletion_queue().push([=]() {
            vmaDestroyBuffer(allocator, vk_buffer.handle, vk_buffer.allocation);
        });

        // 4. Destroy the entity in our database, freeing the handle for reuse
        m_resources_db.destroy(handle);
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

        VmaAllocator allocator = m_Context->get_vma_allocator();
        VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &newTexture.handle, &newTexture.allocation,
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

            VK_CHECK(vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer.handle,
                                     &stagingBuffer.allocation, nullptr));

            // Copy data from the application to the staging buffer
            void *mappedData;
            vmaMapMemory(allocator, stagingBuffer.allocation, &mappedData);
            memcpy(mappedData, desc.initialData, static_cast<size_t>(desc.initialDataSize));
            vmaUnmapMemory(allocator, stagingBuffer.allocation);

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
                barrier_to_shader_read.subresourceRange = barrier_to_transfer.subresourceRange;
                // Subresource range is the same
                barrier_to_shader_read.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier_to_shader_read.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                                     nullptr, 0, nullptr, 1, &barrier_to_shader_read);
            });

            // Clean up the temporary staging buffer now that the data is on the GPU
            vmaDestroyBuffer(allocator, stagingBuffer.handle, stagingBuffer.allocation);
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

        VkDevice logicalDevice = m_Context->get_logical_device();
        VK_CHECK(vkCreateImageView(logicalDevice, &viewInfo, nullptr, &newTexture.image_view));

        auto handle = RAL::TextureHandle{m_resources_db.create()};
        m_resources_db.emplace<VulkanTexture>(handle, newTexture);
        m_resources_db.emplace<RAL::TextureDescription>(handle, desc); // Store the recipe!

        return handle;
    }

    void VulkanDevice::destroy_texture(RAL::TextureHandle handle) {
        if (!m_resources_db.is_valid(handle)) return;

        const auto &vk_texture = m_resources_db.get<VulkanTexture>(handle);
        VkDevice logicalDevice = m_Context->get_logical_device();
        VmaAllocator allocator = m_Context->get_vma_allocator();

        get_current_frame_deletion_queue().push([=]() {
            vkDestroyImageView(logicalDevice, vk_texture.image_view, nullptr);
            vmaDestroyImage(allocator, vk_texture.handle, vk_texture.allocation);
        });

        m_resources_db.destroy(handle);
    }

    void VulkanDevice::wait_idle() {
        vkDeviceWaitIdle(m_Context->get_logical_device());
    }

    void VulkanDevice::recreate_swapchain() {
        if (m_Swapchain) {
            // If it exists, recreate it
            m_Swapchain->recreate();
        }
    }
}
