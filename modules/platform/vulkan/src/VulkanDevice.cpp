#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommon.h"
#include "VulkanMappers.h"
#include "VulkanTypes.h"

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

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.pNext = nullptr;
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

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreInfo.pNext = nullptr;
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
        create_swapchain_texture_handles();
        create_depth_textures(); // NEW depth attachments
        RDE_CORE_INFO("Vulkan Device Initialized successfully.");
    }

    VulkanDevice::~VulkanDevice() {
        wait_idle();
        destroy_depth_textures(); // NEW
        destroy_swapchain_texture_handles();
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

    void VulkanDevice::create_swapchain_texture_handles() {
        const auto &swapchainImages = m_Swapchain->get_images();
        const auto &swapchainImageViews = m_Swapchain->get_image_views();
        m_SwapchainTextureHandles.resize(swapchainImages.size());

        for (size_t i = 0; i < swapchainImages.size(); ++i) {
            auto handle = RAL::TextureHandle{m_resources_db.create()};
            VulkanTexture vkTexture;
            vkTexture.handle = swapchainImages[i];
            vkTexture.image_view = swapchainImageViews[i];
            vkTexture.allocation = nullptr; // Not managed by VMA
            vkTexture.currentLayout = RAL::ImageLayout::Undefined; // start undefined per spec
            vkTexture.isSwapchainImage = true; // explicit flag

            RAL::TextureDescription desc;
            desc.width = m_Swapchain->get_extent().width;
            desc.height = m_Swapchain->get_extent().height;
            desc.format = ToRALFormat(m_Swapchain->get_image_format());
            desc.usage = RAL::TextureUsage::ColorAttachment;

            m_resources_db.emplace<VulkanTexture>(handle, vkTexture);
            m_resources_db.emplace<RAL::TextureDescription>(handle, desc);
            m_SwapchainTextureHandles[i] = handle;
        }
    }

    void VulkanDevice::destroy_swapchain_texture_handles() {
        for (auto &handle: m_SwapchainTextureHandles) {
            if (m_resources_db.is_valid(handle)) {
                // Don't destroy the underlying image/view, just the entity
                m_resources_db.destroy(handle);
            }
        }
        m_SwapchainTextureHandles.clear();
    }

    void VulkanDevice::create_depth_textures() { // NEW
        destroy_depth_textures();
        auto extent = m_Swapchain->get_extent();
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT; // Could add selection logic
        m_SwapchainDepthTextureHandles.resize(m_SwapchainTextureHandles.size());
        for(size_t i=0;i<m_SwapchainDepthTextureHandles.size();++i){
            RAL::TextureDescription d{}; d.width = extent.width; d.height = extent.height; d.depth = 1; d.mipLevels = 1; d.format = ToRALFormat(depthFormat); d.usage = RAL::TextureUsage::DepthStencilAttachment;
            auto tex = create_texture(d);
            auto &vkTex = m_resources_db.get<VulkanTexture>(tex);
            vkTex.currentLayout = RAL::ImageLayout::Undefined; // will transition on first use
            m_SwapchainDepthTextureHandles[i] = tex;
        }
    }

    void VulkanDevice::destroy_depth_textures() { // NEW
        for(auto &h : m_SwapchainDepthTextureHandles){
            if(h.is_valid()) destroy_texture(h);
        }
        m_SwapchainDepthTextureHandles.clear();
    }

    RAL::FrameContext VulkanDevice::begin_frame() {
        auto logicalDevice = m_Context->get_logical_device();
        VK_CHECK(vkWaitForFences(logicalDevice, 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX));

        uint32_t imageIndex;
        VkResult result = m_Swapchain->acquire_next_image(m_ImageAvailableSemaphores[m_CurrentFrameIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            recreate_swapchain();
            return {RAL::TextureHandle::INVALID(), 0, 0, RAL::TextureHandle::INVALID()}; // Return an invalid context
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        // remove forced layout reset; rely on tracked currentLayout (Undefined first use, PresentSrc thereafter)

        // Now we can safely reset the fence as we are about to use its frame resources
        VK_CHECK(vkResetFences(logicalDevice, 1, &m_InFlightFences[m_CurrentFrameIndex]));
        get_current_frame_deletion_queue().flush();

        // Reset the command buffer for this frame so it can be recorded into
        VulkanCommandBuffer *cmd = m_FrameCommandBuffers[m_CurrentFrameIndex].get();
        VK_CHECK(vkResetCommandBuffer(cmd->get_handle(), 0));

        return {m_SwapchainTextureHandles[imageIndex], m_CurrentFrameIndex, imageIndex, m_SwapchainDepthTextureHandles.empty()? RAL::TextureHandle::INVALID() : m_SwapchainDepthTextureHandles[imageIndex]};
    }

    void VulkanDevice::end_frame(const RAL::FrameContext &context,
                                 const std::vector<RAL::CommandBuffer *> &command_buffers) {
        if (!context.swapchainTexture.is_valid()) {
            // Frame was skipped (e.g., minimized window), so just advance the frame index
            m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % FRAMES_IN_FLIGHT;
            return;
        }

        std::vector<VkCommandBuffer> vkCommandBuffers;
        vkCommandBuffers.reserve(command_buffers.size()+1);
        for (const auto &cmd: command_buffers) {
            vkCommandBuffers.push_back(dynamic_cast<VulkanCommandBuffer *>(cmd)->get_handle());
        }

        // SAFETY: Ensure the swapchain image is back to PRESENT layout before presenting.
        // If user code (or an unexpected path) failed to transition it, inject a late barrier.
        if(m_resources_db.is_valid(context.swapchainTexture)) {
            auto &vkTex = m_resources_db.get<VulkanTexture>(context.swapchainTexture);
            if(vkTex.currentLayout != RAL::ImageLayout::PresentSrc) {
                RDE_CORE_WARN("Swapchain image not in PRESENT layout at end_frame (tracked={}, imageIndex={}). Injecting fix-up barrier.", (int)vkTex.currentLayout, context.swapchainImageIndex);
                // Allocate a transient command buffer to perform the transition
                VkCommandBufferAllocateInfo allocInfo;
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = m_CommandPool;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandBufferCount = 1;
                VkCommandBuffer fixupCmd{};
                VK_CHECK(vkAllocateCommandBuffers(m_Context->get_logical_device(), &allocInfo, &fixupCmd));
                VkCommandBufferBeginInfo beginInfo;
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                VK_CHECK(vkBeginCommandBuffer(fixupCmd, &beginInfo));
                VkImageMemoryBarrier b{};
                b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                b.image = vkTex.handle;
                b.oldLayout = ToVulkanImageLayout(vkTex.currentLayout);
                b.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                b.subresourceRange.baseMipLevel = 0; b.subresourceRange.levelCount = 1;
                b.subresourceRange.baseArrayLayer = 0; b.subresourceRange.layerCount = 1;
                // Assume last use was color attachment
                b.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                b.dstAccessMask = 0;
                vkCmdPipelineBarrier(
                    fixupCmd,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    0, 0,nullptr,0,nullptr,1,&b
                );
                VK_CHECK(vkEndCommandBuffer(fixupCmd));
                vkCommandBuffers.push_back(fixupCmd);
                vkTex.currentLayout = RAL::ImageLayout::PresentSrc; // update tracking
            }
        }

        submit_internal(vkCommandBuffers);

        VkResult result = m_Swapchain->present(
                m_RenderFinishedSemaphores[m_CurrentFrameIndex],
                m_Context->get_present_queue(),
                context.swapchainImageIndex // Use the index from the context!
        );
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            recreate_swapchain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % FRAMES_IN_FLIGHT;
    }

    RAL::CommandBuffer *VulkanDevice::get_command_buffer() {
        return m_FrameCommandBuffers[m_CurrentFrameIndex].get();
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
            RDE_CORE_ERROR("Attempted to update an invalid buffer handle.");
            return;
        }

        const auto &target_desc = m_resources_db.get<RAL::BufferDescription>(target_handle);
        const auto &target_vk_buffer = m_resources_db.get<VulkanBuffer>(target_handle);

        // --- PATH 1: The buffer is directly mappable by the CPU ---
        if (target_desc.memoryUsage == RAL::MemoryUsage::HostVisibleCoherent) {
            // VMA can create persistently mapped buffers. If we have a pointer, use it.
            // This is the fastest path for frequent updates.
            if (target_vk_buffer.mapped_data) {
                memcpy(static_cast<uint8_t *>(target_vk_buffer.mapped_data) + offset, data, size);
            } else {
                // Fallback for non-persistently mapped buffers
                void *mapped_data = this->map_buffer(target_handle);
                if (mapped_data) {
                    memcpy(static_cast<uint8_t *>(mapped_data) + offset, data, size);
                    this->unmap_buffer(target_handle);
                }
            }
        }
            // --- PATH 2: The buffer is on the GPU, requiring a staging transfer ---
        else {
            // Create a temporary staging buffer using our own RAL interface.
            RAL::BufferDescription staging_desc = {};
            staging_desc.size = size;
            staging_desc.usage = RAL::BufferUsage::TransferSrc;
            staging_desc.memoryUsage = RAL::MemoryUsage::HostVisibleCoherent;
            RAL::BufferHandle staging_handle = this->create_buffer(staging_desc);

            // Map the staging buffer and copy the user's data into it.
            void *mapped_data = this->map_buffer(staging_handle);
            assert(mapped_data != nullptr && "Failed to map staging buffer");
            memcpy(mapped_data, data, size);
            this->unmap_buffer(staging_handle);

            // Use the clean, public immediate_submit to perform the GPU-side copy.
            this->immediate_submit([&](RAL::CommandBuffer *cmd) {
                // Record a copy from the staging buffer to the final destination buffer.
                // We are using the clean RAL command here, with RAL handles.
                cmd->copy_buffer(staging_handle, target_handle, size, 0, offset);
            });

            // Clean up the temporary staging buffer.
            this->destroy_buffer(staging_handle);
        }
    }

    void VulkanDevice::immediate_submit(std::function<void(RAL::CommandBuffer *cmd)> &&function) {
        VkDevice logicalDevice = m_Context->get_logical_device();
        VkQueue graphicsQueue = m_Context->get_graphics_queue();

        // The key change: Create a temporary RAL-compliant command buffer wrapper
        // for the user's lambda.
        VulkanCommandBuffer upload_cmd_wrapper(m_UploadCommandBuffer, this);

        // Begin the native command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(m_UploadCommandBuffer, &beginInfo));

        // Call the user's function, passing them our clean RAL interface wrapper
        function(&upload_cmd_wrapper);

        // End the native command buffer
        VK_CHECK(vkEndCommandBuffer(m_UploadCommandBuffer));

        // --- The submission logic remains the same ---
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_UploadCommandBuffer;
        VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_UploadFence));

        VK_CHECK(vkWaitForFences(logicalDevice, 1, &m_UploadFence, VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(logicalDevice, 1, &m_UploadFence));

        VK_CHECK(vkResetCommandPool(logicalDevice, m_UploadCommandPool, 0));
    }

    void VulkanDevice::submit_and_wait(const std::vector<RAL::CommandBuffer *> &command_buffers) {
        if (command_buffers.empty()) {
            return;
        }

        VkDevice logicalDevice = m_Context->get_logical_device();
        VkQueue graphicsQueue = m_Context->get_graphics_queue();

        // Convert our RAL command buffers to native Vulkan handles
        std::vector<VkCommandBuffer> vkCommandBuffers;
        vkCommandBuffers.reserve(command_buffers.size());
        for (const auto &cmd: command_buffers) {
            // We know our implementation is VulkanCommandBuffer, so a static_cast is safe and fast.
            vkCommandBuffers.push_back(static_cast<VulkanCommandBuffer *>(cmd)->get_handle());
        }

        // Submit the work using our dedicated upload fence
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = static_cast<uint32_t>(vkCommandBuffers.size());
        submitInfo.pCommandBuffers = vkCommandBuffers.data();
        VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_UploadFence));

        // Wait for the GPU to finish all the work
        VK_CHECK(vkWaitForFences(logicalDevice, 1, &m_UploadFence, VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(logicalDevice, 1, &m_UploadFence));

        // NOTE: We don't reset the command pool here because the command buffers
        // were allocated from the main pool, not the upload pool. The caller
        // is responsible for managing their lifetime (e.g., getting them from `get_command_buffer`).
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
        auto logicalDevice = m_Context->get_logical_device();
        VulkanPipeline newPipeline; // Will hold the final VkPipeline and VkPipelineLayout
        // --- 1. Create Pipeline Layout (unchanged) ---
        {
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
        }

        // --- 2. Create the Specific Pipeline using std::visit ---
        // This is the core of the new logic. We visit the 'stages' variant.
        std::visit([&](auto &&stages) {
            using T = std::decay_t<decltype(stages)>;
            if constexpr (std::is_same_v<T, RAL::GraphicsShaderStages>) {
                std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
                auto add_shader_stage = [&](RAL::ShaderHandle handle, VkShaderStageFlagBits stage) {
                    if (handle.is_valid()) {
                        const auto &shader = m_resources_db.get<VulkanShader>(handle);
                        VkPipelineShaderStageCreateInfo stageInfo{}; // zero init
                        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                        stageInfo.stage = stage;
                        stageInfo.module = shader.module;
                        stageInfo.pName = "main";
                        shaderStages.push_back(stageInfo);
                    }
                };
                add_shader_stage(stages.vertexShader, VK_SHADER_STAGE_VERTEX_BIT);
                add_shader_stage(stages.fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT);
                add_shader_stage(stages.geometryShader, VK_SHADER_STAGE_GEOMETRY_BIT);
                add_shader_stage(stages.tessControlShader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
                add_shader_stage(stages.tessEvalShader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

                // Vertex Input
                std::vector<VkVertexInputBindingDescription> bindings;
                for (const auto &b: desc.vertexBindings) {
                    VkVertexInputRate rate = (b.inputRate == RAL::VertexInputBinding::Rate::PerVertex)
                                             ? VK_VERTEX_INPUT_RATE_VERTEX
                                             : VK_VERTEX_INPUT_RATE_INSTANCE;
                    bindings.push_back({b.binding, b.stride, rate});
                }
                std::vector<VkVertexInputAttributeDescription> attributes;
                for (const auto &a: desc.vertexAttributes) {
                    attributes.push_back({a.location, a.binding, ToVulkanFormat(a.format), a.offset});
                }

                VkPipelineVertexInputStateCreateInfo vertexInputInfo{}; vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
                vertexInputInfo.pVertexBindingDescriptions = bindings.data();
                vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
                vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

                VkPipelineInputAssemblyStateCreateInfo inputAssembly{}; inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssembly.topology = ToVulkanPrimitive(desc.topology);
                inputAssembly.primitiveRestartEnable = VK_FALSE;

                VkPipelineViewportStateCreateInfo viewportState{}; viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                viewportState.viewportCount = 1; viewportState.scissorCount = 1;

                std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
                VkPipelineDynamicStateCreateInfo dynamicState{}; dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
                dynamicState.pDynamicStates = dynamicStates.data();

                VkPipelineRasterizationStateCreateInfo rasterizer{}; rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterizer.depthClampEnable = VK_FALSE; rasterizer.rasterizerDiscardEnable = VK_FALSE;
                rasterizer.polygonMode = ToVulkanPolygonMode(desc.rasterizationState.polygonMode);
                rasterizer.lineWidth = 1.0f;
                rasterizer.cullMode = ToVulkanCullMode(desc.rasterizationState.cullMode);
                rasterizer.frontFace = ToVulkanFrontFace(desc.rasterizationState.frontFace);
                rasterizer.depthBiasEnable = VK_FALSE;

                VkPipelineMultisampleStateCreateInfo multisampling{}; multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisampling.sampleShadingEnable = VK_FALSE; multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

                VkPipelineDepthStencilStateCreateInfo depthStencil{}; depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                VkPipelineDepthStencilStateCreateInfo *pDepthStencilState = nullptr;
                if (desc.depthAttachmentFormat != RAL::Format::UNKNOWN) {
                    // Always provide a depth-stencil state when a depth (or stencil) attachment format is specified,
                    // even if tests are disabled (spec requirement for dynamic rendering without dynamic state3).
                    depthStencil.depthTestEnable = desc.depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE;
                    depthStencil.depthWriteEnable = desc.depthStencilState.depthWriteEnable ? VK_TRUE : VK_TRUE; // must be valid; if disabled writeEnable ignored
                    depthStencil.depthCompareOp = ToVulkanCompareOp(desc.depthStencilState.depthCompareOp);
                    depthStencil.depthBoundsTestEnable = VK_FALSE;
                    depthStencil.stencilTestEnable = desc.depthStencilState.stencilTestEnable ? VK_TRUE : VK_FALSE;
                    depthStencil.front = {};
                    depthStencil.back = {};
                    pDepthStencilState = &depthStencil;
                }

                VkPipelineColorBlendAttachmentState colorBlendAttachment{};
                colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                colorBlendAttachment.blendEnable = desc.colorBlendState.attachment.blendEnable;
                const auto &ralBlend = desc.colorBlendState.attachment;
                colorBlendAttachment.srcColorBlendFactor = ToVulkanBlendFactor(ralBlend.srcColorBlendFactor);
                colorBlendAttachment.dstColorBlendFactor = ToVulkanBlendFactor(ralBlend.dstColorBlendFactor);
                colorBlendAttachment.colorBlendOp = ToVulkanBlendOp(ralBlend.colorBlendOp);
                colorBlendAttachment.srcAlphaBlendFactor = ToVulkanBlendFactor(ralBlend.srcAlphaBlendFactor);
                colorBlendAttachment.dstAlphaBlendFactor = ToVulkanBlendFactor(ralBlend.dstAlphaBlendFactor);
                colorBlendAttachment.alphaBlendOp = ToVulkanBlendOp(ralBlend.alphaBlendOp);
                VkPipelineColorBlendStateCreateInfo colorBlending{}; colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                colorBlending.logicOpEnable = VK_FALSE; colorBlending.attachmentCount = 1; colorBlending.pAttachments = &colorBlendAttachment;

                std::vector<VkFormat> colorFormats;
                if (!desc.colorAttachmentFormats.empty()) {
                    for (auto f : desc.colorAttachmentFormats) colorFormats.push_back(ToVulkanFormat(f));
                } else { colorFormats.push_back(m_Swapchain->get_image_format()); }
                VkFormat depthFormat = VK_FORMAT_UNDEFINED;
                if (desc.depthAttachmentFormat != RAL::Format::UNKNOWN) depthFormat = ToVulkanFormat(desc.depthAttachmentFormat);
                VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{}; pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
                pipelineRenderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
                pipelineRenderingCreateInfo.pColorAttachmentFormats = colorFormats.data();
                pipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;
                // Only supply stencil attachment format if depth format actually has stencil aspect
                if(depthFormat == VK_FORMAT_D24_UNORM_S8_UINT || depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) {
                    pipelineRenderingCreateInfo.stencilAttachmentFormat = depthFormat;
                } else {
                    pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
                }

                VkGraphicsPipelineCreateInfo pipelineInfo{}; pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipelineInfo.pNext = &pipelineRenderingCreateInfo;
                pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
                pipelineInfo.pStages = shaderStages.data();
                pipelineInfo.layout = newPipeline.layout;
                pipelineInfo.pVertexInputState = &vertexInputInfo;
                pipelineInfo.pInputAssemblyState = &inputAssembly;
                pipelineInfo.pViewportState = &viewportState;
                pipelineInfo.pRasterizationState = &rasterizer;
                pipelineInfo.pMultisampleState = &multisampling;
                pipelineInfo.pDepthStencilState = pDepthStencilState;
                pipelineInfo.pColorBlendState = &colorBlending;
                pipelineInfo.pDynamicState = &dynamicState;
                pipelineInfo.subpass = 0;
                VK_CHECK(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline.handle));
                newPipeline.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            } else if constexpr (std::is_same_v<T, RAL::ComputeShaderStages>) {
                VkComputePipelineCreateInfo pipelineInfo{}; pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                pipelineInfo.layout = newPipeline.layout;
                const auto &shader = m_resources_db.get<VulkanShader>(stages.computeShader);
                VkPipelineShaderStageCreateInfo stageInfo{}; stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; stageInfo.module = shader.module; stageInfo.pName = "main";
                pipelineInfo.stage = stageInfo;

                VK_CHECK(vkCreateComputePipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline.handle));
                newPipeline.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
            } else if constexpr (std::is_same_v<T, RAL::MeshShaderStages>) {
                RDE_CORE_ERROR("Mesh shader pipelines are not yet implemented!");
            }

        }, desc.stages);


        // --- 3. Store the result in the database (Common to all types) ---
        if (!newPipeline.handle) {
            // If handle is null, creation failed. Clean up the layout.
            vkDestroyPipelineLayout(logicalDevice, newPipeline.layout, nullptr);
            return RAL::PipelineHandle::INVALID();
        }

        auto handle = RAL::PipelineHandle{m_resources_db.create()};
        m_resources_db.emplace<VulkanPipeline>(handle, newPipeline);
        m_resources_db.emplace<RAL::PipelineDescription>(handle, desc);

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
        // Hash descriptor set layout
        size_t h = std::hash<uint32_t>()(desc.set);
        for(const auto &b : desc.bindings){
            size_t bh = ((size_t)b.binding<<1) ^ (size_t)b.type ^ (size_t)ToVulkanShaderStageFlags(b.stages);
            h ^= (bh + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2));
        }
        if(auto it = m_DescriptorSetLayoutCache.find(h); it != m_DescriptorSetLayoutCache.end()){
            it->second.refCount++; return it->second.handle;
        }
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

        // store in cache
        m_DescriptorSetLayoutCache.emplace(h, CachedLayoutEntry{handle,1,desc});
        return handle;
    }

    void VulkanDevice::destroy_descriptor_set_layout(RAL::DescriptorSetLayoutHandle handle) {
        if (!m_resources_db.is_valid(handle)) return;
        // locate in cache and decrement
        for(auto it = m_DescriptorSetLayoutCache.begin(); it != m_DescriptorSetLayoutCache.end(); ++it){
            if(it->second.handle == handle){
                if(--it->second.refCount > 0){
                    m_resources_db.destroy(handle); return; // still in use
                } else {
                    // proceed to destroy
                    m_DescriptorSetLayoutCache.erase(it);
                    break;
                }
            }
        }
        const auto &vk_descriptor_set_layout = m_resources_db.get<VulkanDescriptorSetLayout>(handle);
        VkDevice logicalDevice = m_Context->get_logical_device();
        get_current_frame_deletion_queue().push([=]() { vkDestroyDescriptorSetLayout(logicalDevice, vk_descriptor_set_layout.handle, nullptr); });
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
                case RAL::DescriptorType::Sampler: {
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
        if (desc.memoryUsage == RAL::MemoryUsage::HostVisibleCoherent) {
            // Automatically map persistent host-visible buffers. Big performance win.
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        VmaAllocator allocator = m_Context->get_vma_allocator();
        VulkanBuffer newBuffer;
        VmaAllocationInfo vmaAllocInfo{};
        VK_CHECK(
                vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &newBuffer.handle, &newBuffer.allocation,
                                &vmaAllocInfo)
        );
        // Store the persistently mapped pointer if we got one.
        newBuffer.mapped_data = vmaAllocInfo.pMappedData;

        auto handle = RAL::BufferHandle{this->m_resources_db.create()};
        m_resources_db.emplace<RAL::BufferDescription>(handle, desc);
        m_resources_db.emplace<VulkanBuffer>(handle, newBuffer);
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
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = {desc.width, desc.height, 1};
        imageInfo.mipLevels = desc.mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = ToVulkanFormat(desc.format);
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = ToVulkanImageUsage(desc.usage);
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        VmaAllocator allocator = m_Context->get_vma_allocator();
        VK_CHECK(
                vmaCreateImage(allocator, &imageInfo, &allocInfo, &newTexture.handle, &newTexture.allocation, nullptr));

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = newTexture.handle;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = ToVulkanFormat(desc.format);
        viewInfo.subresourceRange.aspectMask = (has_flag(desc.usage, RAL::TextureUsage::DepthStencilAttachment))
                                               ? VK_IMAGE_ASPECT_DEPTH_BIT
                                               : VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = desc.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkDevice logicalDevice = m_Context->get_logical_device();
        VK_CHECK(vkCreateImageView(logicalDevice, &viewInfo, nullptr, &newTexture.image_view));

        auto handle = RAL::TextureHandle{m_resources_db.create()};
        m_resources_db.emplace<VulkanTexture>(handle, newTexture);
        m_resources_db.emplace<RAL::TextureDescription>(handle, desc);
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
            destroy_depth_textures();
            destroy_swapchain_texture_handles();
            m_Swapchain->recreate();
            create_swapchain_texture_handles();
            create_depth_textures();
        }
    }
}
