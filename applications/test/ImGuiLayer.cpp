#include "ImGuiLayer.h"
#include "ral/Resources.h"
#include "core/Application.h"
#include "core/Paths.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace RDE {
    ImGuiLayer::ImGuiLayer(IWindow *window, RAL::Device *device) : m_window(window), m_device(device) {
    }

    ImGuiLayer::~ImGuiLayer() {
        if (m_Context) {
            on_detach();
        }
    }

    void ImGuiLayer::on_attach() {
        // This is where we will create OUR resources.
        IMGUI_CHECKVERSION();
        // 1. Setup ImGui Context and Platform Backend
        m_Context = ImGui::CreateContext();
        ImGui::SetCurrentContext(m_Context);

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        io.DisplaySize = ImVec2((float) m_window->get_width(),
                                (float) m_window->get_height());

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow *>(m_window->get_native_handle()), true);

        // 2. NOW, WE BUILD OUR OWN RENDERER BACKEND
        create_ral_resources();
    }

    void ImGuiLayer::on_detach() {
        if (!m_device) return;
        m_device->wait_idle();

        destroy_ral_resources(); // Our own cleanup function

        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(m_Context);
        m_Context = nullptr;
    }

    void ImGuiLayer::on_update([[maybe_unused]] float delta_time) {
    }

    void ImGuiLayer::on_render_gui() {
        ImGui::ShowDemoWindow();
    }

    void ImGuiLayer::on_event([[maybe_unused]] Event &event) {
    }

    void ImGuiLayer::begin() {
        ImGui_ImplGlfw_NewFrame(); // Let ImGui process input from the window
        ImGui::NewFrame();
        ImGui::BeginMainMenuBar();
    }

    void ImGuiLayer::end(RAL::CommandBuffer *cmd) {
        ImGui::EndMainMenuBar();
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();

        if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f) {
            return;
        }

        // 1. Update/Create Vertex and Index Buffers
        size_t vb_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
        if (vb_size > m_VertexBufferSize) {
            if (m_VertexBuffer.is_valid()) m_device->destroy_buffer(m_VertexBuffer);
            RAL::BufferDescription desc{};
            desc.size = vb_size * 1.5;
            desc.usage = RAL::BufferUsage::VertexBuffer;
            desc.memoryUsage = RAL::MemoryUsage::HostVisibleCoherent; // IMPORTANT
            m_VertexBuffer = m_device->create_buffer(desc);
            m_VertexBufferSize = desc.size;
        }

        size_t ib_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
        if (ib_size > m_IndexBufferSize) {
            if (m_IndexBuffer.is_valid()) m_device->destroy_buffer(m_IndexBuffer);
            RAL::BufferDescription desc{};
            desc.size = ib_size * 1.5;
            desc.usage = RAL::BufferUsage::IndexBuffer;
            desc.memoryUsage = RAL::MemoryUsage::HostVisibleCoherent;
            m_IndexBuffer = m_device->create_buffer(desc);
            m_IndexBufferSize = desc.size;
        }

        // 2. Upload data using our mapping functions
        if (vb_size > 0 && ib_size > 0) {
            auto *vtx_dst = (ImDrawVert *) m_device->map_buffer(m_VertexBuffer);
            auto *idx_dst = (ImDrawIdx *) m_device->map_buffer(m_IndexBuffer);

            for (int n = 0; n < draw_data->CmdListsCount; n++) {
                const ImDrawList *cmd_list = draw_data->CmdLists[n];
                memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtx_dst += cmd_list->VtxBuffer.Size;
                idx_dst += cmd_list->IdxBuffer.Size;
            }
            m_device->unmap_buffer(m_VertexBuffer);
            m_device->unmap_buffer(m_IndexBuffer);
        } else {
            return; // Nothing to render
        }

        // 3. Record Draw Commands
        setup_render_state(draw_data, cmd, (int) draw_data->DisplaySize.x, (int) draw_data->DisplaySize.y);
    }

    struct ImGuiPushConstants {
        glm::vec2 scale;
        glm::vec2 translate;
    };

    void ImGuiLayer::setup_render_state(ImDrawData *draw_data, RAL::CommandBuffer *cmd, int fb_width, int fb_height) {
        cmd->bind_pipeline(m_Pipeline);
        cmd->bind_descriptor_set(m_Pipeline, m_DescriptorSet, 0); // Bind our texture
        cmd->bind_vertex_buffer(m_VertexBuffer, 0);
        cmd->bind_index_buffer(m_IndexBuffer, sizeof(ImDrawIdx) == 2 ? RAL::IndexType::UINT16 : RAL::IndexType::UINT32);

        RAL::Viewport viewport{};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(fb_width);
        viewport.height = static_cast<float>(fb_height);
        viewport.min_depth = 0.0f;
        viewport.max_depth = 1.0f;
        cmd->set_viewport(viewport);

        ImGuiPushConstants push_constants;
        push_constants.scale.x = 2.0f / draw_data->DisplaySize.x;
        push_constants.scale.y = 2.0f / draw_data->DisplaySize.y;
        push_constants.translate.x = -1.0f - draw_data->DisplayPos.x * push_constants.scale.x;
        push_constants.translate.y = -1.0f - draw_data->DisplayPos.y * push_constants.scale.y;

        cmd->push_constants(m_Pipeline, RAL::ShaderStage::Vertex, 0, sizeof(ImGuiPushConstants), &push_constants);

        // Render command lists
        int global_vtx_offset = 0;
        int global_idx_offset = 0;
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList *cmd_list = draw_data->CmdLists[n];
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];

                // Setup scissor rect
                RAL::Rect2D scissor;
                scissor.x = (int32_t) pcmd->ClipRect.x > 0 ? (int32_t) pcmd->ClipRect.x : 0;
                scissor.y = (int32_t) pcmd->ClipRect.y > 0 ? (int32_t) pcmd->ClipRect.y : 0;
                scissor.width = (uint32_t) (pcmd->ClipRect.z - pcmd->ClipRect.x);
                scissor.height = (uint32_t) (pcmd->ClipRect.w - pcmd->ClipRect.y);
                cmd->set_scissor(scissor);

                cmd->draw_indexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset,
                                  pcmd->VtxOffset + global_vtx_offset, 0);
            }
            global_idx_offset += cmd_list->IdxBuffer.Size;
            global_vtx_offset += cmd_list->VtxBuffer.Size;
        }
    }

    void ImGuiLayer::create_ral_resources() {
        ImGuiIO &io = ImGui::GetIO();

        // === 1. Create Font Texture and Sampler ===
        unsigned char *pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        size_t upload_size = width * height * 4 * sizeof(char);

        // 1a. Create the FINAL destination texture on the GPU. It starts empty.
        RAL::TextureDescription fontDesc{};
        fontDesc.width = width;
        fontDesc.height = height;
        fontDesc.format = RAL::Format::R8G8B8A8_UNORM;
        // IMPORTANT: It needs to be a Sampled texture AND a Transfer Destination.
        fontDesc.usage = RAL::TextureUsage::Sampled | RAL::TextureUsage::TransferDst;
        m_FontTexture = m_device->create_texture(fontDesc);

        // 1b. Create a temporary, CPU-visible staging buffer for the upload.
        RAL::BufferDescription stagingDesc{};
        stagingDesc.size = upload_size;
        stagingDesc.usage = RAL::BufferUsage::TransferSrc;
        stagingDesc.memoryUsage = RAL::MemoryUsage::HostVisibleCoherent;
        RAL::BufferHandle stagingBuffer = m_device->create_buffer(stagingDesc);

        // 1c. Map the staging buffer and copy the font pixel data into it.
        void *mappedData = m_device->map_buffer(stagingBuffer);
        memcpy(mappedData, pixels, upload_size);
        m_device->unmap_buffer(stagingBuffer);

        // 1d. Use an immediate command buffer to perform the GPU-side copy.
        // This is a blocking operation, which is fine for one-time setup.

        m_device->immediate_submit([&](RAL::CommandBuffer *cmd) {
            // Transition the destination texture to be ready for the copy
            RAL::ResourceBarrier barrier_to_transfer_dst = {};
            barrier_to_transfer_dst.srcStage = RAL::PipelineStageFlags::TopOfPipe;
            barrier_to_transfer_dst.srcAccess = RAL::AccessFlags::None;
            barrier_to_transfer_dst.dstStage = RAL::PipelineStageFlags::Transfer;
            barrier_to_transfer_dst.dstAccess = RAL::AccessFlags::TransferWrite;
            barrier_to_transfer_dst.textureTransition = {
                m_FontTexture, RAL::ImageLayout::Undefined, RAL::ImageLayout::TransferDst
            };
            cmd->pipeline_barrier(barrier_to_transfer_dst);

            // Record the copy command
            RAL::BufferTextureCopy copyRegion = {};
            copyRegion.imageExtent = {(uint32_t) width, (uint32_t) height, 1};
            cmd->copy_buffer_to_texture(stagingBuffer, m_FontTexture, {copyRegion});

            // Transition the texture to be ready for shader sampling
            RAL::ResourceBarrier barrier_to_shader_read = {};
            barrier_to_shader_read.srcStage = RAL::PipelineStageFlags::Transfer;
            barrier_to_shader_read.srcAccess = RAL::AccessFlags::TransferWrite;
            barrier_to_shader_read.dstStage = RAL::PipelineStageFlags::FragmentShader;
            barrier_to_shader_read.dstAccess = RAL::AccessFlags::ShaderRead;
            barrier_to_shader_read.textureTransition = {
                m_FontTexture, RAL::ImageLayout::TransferDst, RAL::ImageLayout::ShaderReadOnly
            };
            cmd->pipeline_barrier(barrier_to_shader_read);
        });


        // 1e. Clean up the temporary staging buffer. Its job is done.
        m_device->destroy_buffer(stagingBuffer);

        io.Fonts->SetTexID((ImTextureID) (intptr_t) m_FontTexture.index); // Use the handle index as an ID

        RAL::SamplerDescription samplerDesc{}; // You need to define this in your RAL
        // samplerDesc.magFilter = ...;
        // samplerDesc.minFilter = ...;
        m_FontSampler = m_device->create_sampler(samplerDesc);

        // === 2. Create Descriptor Set Layout ===
        RAL::DescriptorSetLayoutDescription layoutDesc{};
        layoutDesc.bindings.push_back({
            .binding = 0,
            .type = RAL::DescriptorType::CombinedImageSampler,
            .stages = RAL::ShaderStage::Fragment,
            .name = "FontTexture"
        });

        m_DsLayout = m_device->create_descriptor_set_layout(layoutDesc);
        // === 3. Create Descriptor Set ===
        RAL::DescriptorSetDescription setDesc{};
        setDesc.layout = m_DsLayout;
        setDesc.writes.push_back({
            .binding = 0,
            .type = RAL::DescriptorType::CombinedImageSampler,
            .buffer = RAL::BufferHandle::INVALID(),
            .texture = m_FontTexture,
            .sampler = m_FontSampler
        });
        m_DescriptorSet = m_device->create_descriptor_set(setDesc);

        // === 4. Create Pipeline ===
        auto shaderDir = get_shaders_path();
        if (!shaderDir.has_value()) {
            RDE_CORE_ERROR("Failed to get shader directory for ImGuiLayer");
            return;
        }

        std::string vertPath = (shaderDir.value() / "spirv" / "imgui.vert.spv").string();
        std::string fragPath = (shaderDir.value() / "spirv" / "imgui.frag.spv").string();
        RAL::ShaderDescription vsDesc{vertPath, RAL::ShaderStage::Vertex};
        RAL::ShaderDescription fsDesc{fragPath, RAL::ShaderStage::Fragment};
        RAL::ShaderHandle vs = m_device->create_shader(vsDesc);
        RAL::ShaderHandle fs = m_device->create_shader(fsDesc);

        RAL::PipelineDescription psoDesc{};
        psoDesc.vertexShader = vs;
        psoDesc.fragmentShader = fs;
        psoDesc.descriptorSetLayouts.push_back(m_DsLayout); // Use the layout we created

        // Push Constants for scale/translate matrix
        psoDesc.pushConstantRanges.push_back({
            .stages = RAL::ShaderStage::Vertex,
            .offset = 0,
            .size = sizeof(float) * 4,
            .name = "Empty PushConstant"
        });

        // Setup for alpha blending
        // Setup for alpha blending
        psoDesc.colorBlendState.attachment.blendEnable = true;

        // CORRECT BLEND FACTOR FOR NON-PREMULTIPLIED ALPHA
        psoDesc.colorBlendState.attachment.srcColorBlendFactor = RAL::BlendFactor::SrcAlpha;

        psoDesc.colorBlendState.attachment.dstColorBlendFactor = RAL::BlendFactor::OneMinusSrcAlpha;
        psoDesc.colorBlendState.attachment.colorBlendOp = RAL::BlendOp::Add;
        psoDesc.colorBlendState.attachment.srcAlphaBlendFactor = RAL::BlendFactor::One;
        psoDesc.colorBlendState.attachment.dstAlphaBlendFactor = RAL::BlendFactor::OneMinusSrcAlpha;
        psoDesc.colorBlendState.attachment.alphaBlendOp = RAL::BlendOp::Add;

        // Disable culling and depth testing
        psoDesc.rasterizationState.cullMode = RAL::CullMode::None;
        psoDesc.rasterizationState.polygonMode = RAL::PolygonMode::Fill;

        psoDesc.depthStencilState.depthTestEnable = false;
        psoDesc.depthStencilState.depthWriteEnable = false;
        psoDesc.depthStencilState.depthCompareOp = RAL::CompareOp::Always;

        // ImGui vertex layout
        psoDesc.vertexBindings = {{.binding = 0, .stride = sizeof(ImDrawVert)}};
        psoDesc.vertexAttributes = {
            {
                .location = 0, .binding = 0, .format = RAL::Format::R32G32_SFLOAT, .offset = offsetof(ImDrawVert,
                    pos), .name = "in_Position"
            },
            {.location = 1, .binding = 0, .format = RAL::Format::R32G32_SFLOAT, .offset = offsetof(ImDrawVert, uv), .name = "in_TexCoord"},
            {
                .location = 2, .binding = 0, .format = RAL::Format::R8G8B8A8_UNORM, .offset = offsetof(ImDrawVert,
                    col), .name = "in_Color"
            }
        };
        m_Pipeline = m_device->create_pipeline(psoDesc);

        // Cleanup temporary handles
        m_device->destroy_shader(vs);
        m_device->destroy_shader(fs);
        // Don't destroy the layout handle yet if the pipeline needs it
    }

    void ImGuiLayer::destroy_ral_resources() {
        m_device->destroy_pipeline(m_Pipeline);
        m_device->destroy_descriptor_set(m_DescriptorSet);
        m_device->destroy_descriptor_set_layout(m_DsLayout);
        // Need a way to get this
        m_device->destroy_texture(m_FontTexture);
        m_device->destroy_sampler(m_FontSampler);
        if (m_VertexBuffer.is_valid()) m_device->destroy_buffer(m_VertexBuffer);
        if (m_IndexBuffer.is_valid()) m_device->destroy_buffer(m_IndexBuffer);
    }
}
