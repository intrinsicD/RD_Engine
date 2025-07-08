#include  "ImGuiLayer.h"
#include "ral/Resources.h"
#include "core/Application.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

namespace RDE {
    ImGuiLayer::ImGuiLayer(ApplicationContext *app_context) : m_app_context(app_context) {
        m_Device = m_app_context->m_device.get();
    }

    ImGuiLayer::~ImGuiLayer() {
        if (m_Context) {
            on_detach();
        }
    }

    void ImGuiLayer::on_attach() override {
        // This is where we will create OUR resources.

        // 1. Setup ImGui Context and Platform Backend
        m_Context = ImGui::CreateContext();
        ImGui::SetCurrentContext(m_Context);

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        io.DisplaySize = ImVec2((float) m_app_context->m_width, (float) m_app_context->m_height);

        ImGui::StyleColorsDark();

        // Initialize the PLATFORM backend (e.g., GLFW) for input handling.
        // This is OK because it doesn't touch rendering.
        ImGui_ImplGlfw_InitForVulkan(m_app_context->m_window->get_native_handle(), true);

        // 2. NOW, WE BUILD OUR OWN RENDERER BACKEND
        create_ral_resources();
    }

    void ImGuiLayer::on_detach() override {
        if (!m_Device) return;
        m_Device->wait_idle();

        destroy_ral_resources(); // Our own cleanup function

        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(m_Context);
        m_Context = nullptr;
    }

    void ImGuiLayer::on_update(float delta_time) override {
    }

    void ImGuiLayer::on_event(Event &event) override {
    }

    void ImGuiLayer::begin() {
        ImGui_ImplGlfw_NewFrame(); // Let ImGui process input from the window
        ImGui::NewFrame();
    }

    void ImGuiLayer::end(RAL::CommandBuffer &commandBuffer) {
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();

        if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f) {
            return;
        }

        // 1. Update/Create Vertex and Index Buffers
        size_t vb_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
        if (vb_size > m_VertexBufferSize) {
            if (m_VertexBuffer.is_valid()) m_Device->destroy_buffer(m_VertexBuffer);
            RAL::BufferDescription desc{};
            desc.size = vb_size * 1.5;
            desc.usage = RAL::BufferUsage::VertexBuffer;
            desc.memoryUsage = RAL::MemoryUsage::HostVisible; // IMPORTANT
            m_VertexBuffer = m_Device->create_buffer(desc);
            m_VertexBufferSize = desc.size;
        }

        size_t ib_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
        if (ib_size > m_IndexBufferSize) {
            if (m_IndexBuffer.is_valid()) m_Device->destroy_buffer(m_IndexBuffer);
            RAL::BufferDescription desc{};
            desc.size = ib_size * 1.5;
            desc.usage = RAL::BufferUsage::IndexBuffer;
            desc.memoryUsage = RAL::MemoryUsage::HostVisible;
            m_IndexBuffer = m_Device->create_buffer(desc);
            m_IndexBufferSize = desc.size;
        }

        // 2. Upload data using our mapping functions
        if (vb_size > 0 && ib_size > 0) {
            auto *vtx_dst = (ImDrawVert *) m_Device->map_buffer(m_VertexBuffer);
            auto *idx_dst = (ImDrawIdx *) m_Device->map_buffer(m_IndexBuffer);

            for (int n = 0; n < draw_data->CmdListsCount; n++) {
                const ImDrawList *cmd_list = draw_data->CmdLists[n];
                memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtx_dst += cmd_list->VtxBuffer.Size;
                idx_dst += cmd_list->IdxBuffer.Size;
            }
            m_Device->unmap_buffer(m_VertexBuffer);
            m_Device->unmap_buffer(m_IndexBuffer);
        } else {
            return; // Nothing to render
        }

        // 3. Record Draw Commands
        setup_render_state(draw_data, cmd, (int) draw_data->DisplaySize.x, (int) draw_data->DisplaySize.y);
    }

    void ImGuiLayer::setup_render_state(ImDrawData *draw_data, RAL::CommandBuffer *cmd, int fb_width, int fb_height) {
        cmd->bind_pipeline(m_Pipeline);
        cmd->bind_descriptor_set(m_Pipeline, m_DescriptorSet, 0); // Bind our texture
        cmd->bind_vertex_buffer(m_VertexBuffer, 0);
        cmd->bind_index_buffer(m_IndexBuffer, sizeof(ImDrawIdx) == 2 ? RAL::IndexType::UINT16 : RAL::IndexType::UINT32);

        float scale[2] = {2.0f / draw_data->DisplaySize.x, 2.0f / draw_data->DisplaySize.y};
        float translate[2] = {-1.0f - draw_data->DisplayPos.x * scale[0], -1.0f - draw_data->DisplayPos.y * scale[1]};

        float push_data[4] = {scale[0], scale[1], translate[0], translate[1]};
        cmd->push_constants(m_Pipeline, RAL::ShaderStage::Vertex, 0, sizeof(push_data), push_data);

        // Render command lists
        int global_vtx_offset = 0;
        int global_idx_offset = 0;
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList *cmd_list = draw_data->CmdLists[n];
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];

                // Setup scissor rect
                RAL::Rect2D scissor;
                scissor.x = (int32_t) pcmd->ClipRect.x;
                scissor.y = (int32_t) pcmd->ClipRect.y;
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

        RAL::TextureDescription fontDesc{};
        fontDesc.width = width;
        fontDesc.height = height;
        fontDesc.format = RAL::Format::R8G8B8A8_UNORM;
        fontDesc.usage = RAL::TextureUsage::Sampled | RAL::TextureUsage::TransferDst;
        fontDesc.initialData = pixels; // Use the initial data directly
        m_FontTexture = m_Device->create_texture(fontDesc); // Use the version that takes initial data

        io.Fonts->SetTexID((ImTextureID) (intptr_t) m_FontTexture.index); // Use the handle index as an ID

        RAL::SamplerDescription samplerDesc{}; // You need to define this in your RAL
        // samplerDesc.magFilter = ...;
        // samplerDesc.minFilter = ...;
        m_FontSampler = m_Device->create_sampler(samplerDesc);

        // === 2. Create Descriptor Set Layout ===
        RAL::DescriptorSetLayoutDescription layoutDesc{};
        layoutDesc.bindings.push_back({
            .binding = 0,
            .type = RAL::DescriptorType::CombinedImageSampler,
            .stages = RAL::ShaderStage::Fragment
        });
        RAL::DescriptorSetLayoutHandle dsLayout = m_Device->create_descriptor_set_layout(layoutDesc);

        // === 3. Create Descriptor Set ===
        RAL::DescriptorSetDescription setDesc{};
        setDesc.layout = dsLayout;
        setDesc.writes.push_back({
            .binding = 0,
            .type = RAL::DescriptorType::CombinedImageSampler,
            .texture = m_FontTexture,
            .sampler = m_FontSampler
        });
        m_DescriptorSet = m_Device->create_descriptor_set(setDesc);

        // === 4. Create Pipeline ===
        RAL::ShaderDescription vsDesc{"shaders/imgui.vert.spv", RAL::ShaderStage::Vertex};
        RAL::ShaderDescription fsDesc{"shaders/imgui.frag.spv", RAL::ShaderStage::Fragment};
        RAL::ShaderHandle vs = m_Device->create_shader(vsDesc);
        RAL::ShaderHandle fs = m_Device->create_shader(fsDesc);

        RAL::PipelineDescription psoDesc{};
        psoDesc.vertexShader = vs;
        psoDesc.fragmentShader = fs;
        psoDesc.descriptorSetLayouts.push_back(dsLayout); // Use the layout we created

        // Push Constants for scale/translate matrix
        psoDesc.pushConstantRanges.push_back({
            .stages = RAL::ShaderStage::Vertex,
            .offset = 0,
            .size = sizeof(float) * 4
        });

        // Setup for alpha blending
        psoDesc.colorBlendState.attachment.blendEnable = true;
        psoDesc.colorBlendState.attachment.srcColorBlendFactor = RAL::BlendFactor::SrcAlpha;
        psoDesc.colorBlendState.attachment.dstColorBlendFactor = RAL::BlendFactor::OneMinusSrcAlpha;
        psoDesc.colorBlendState.attachment.colorBlendOp = RAL::BlendOp::Add;
        psoDesc.colorBlendState.attachment.srcAlphaBlendFactor = RAL::BlendFactor::One;
        psoDesc.colorBlendState.attachment.dstAlphaBlendFactor = RAL::BlendFactor::OneMinusSrcAlpha;
        psoDesc.colorBlendState.attachment.alphaBlendOp = RAL::BlendOp::Add;

        // Disable culling and depth testing
        psoDesc.rasterizationState.cullMode = RAL::CullMode::None;
        // psoDesc.depthStencilState.depthTestEnable = false;
        // psoDesc.depthStencilState.depthWriteEnable = false;

        // ImGui vertex layout
        psoDesc.vertexBindings = {{.binding = 0, .stride = sizeof(ImDrawVert)}};
        psoDesc.vertexAttributes = {
            {.location = 0, .binding = 0, .format = RAL::Format::R32G32_SFLOAT, .offset = offsetof(ImDrawVert, pos)},
            {.location = 1, .binding = 0, .format = RAL::Format::R32G32_SFLOAT, .offset = offsetof(ImDrawVert, uv)},
            {.location = 2, .binding = 0, .format = RAL::Format::R8G8B8A8_UNORM, .offset = offsetof(ImDrawVert, col)}
        };
        m_Pipeline = m_Device->create_pipeline(psoDesc);

        // Cleanup temporary handles
        m_Device->destroy_shader(vs);
        m_Device->destroy_shader(fs);
        // Don't destroy the layout handle yet if the pipeline needs it
    }

    void ImGuiLayer::destroy_ral_resources() {
        m_Device->destroy_pipeline(m_Pipeline);
        m_Device->destroy_descriptor_set(m_DescriptorSet);
        m_Device->destroy_descriptor_set_layout(m_Device->get_pipeline(m_Pipeline)->GetLayout());
        // Need a way to get this
        m_Device->destroy_texture(m_FontTexture);
        m_Device->destroy_sampler(m_FontSampler);
        if (m_VertexBuffer.is_valid()) m_Device->destroy_buffer(m_VertexBuffer);
        if (m_IndexBuffer.is_valid()) m_Device->destroy_buffer(m_IndexBuffer);
    }

}
