#include "TestSceneLayer.h"
#include "core/Paths.h"
#include "core/FileIOUtils.h"
#include "core/Log.h"
#include "components/TransformComponent.h"
#include "VulkanDevice.h" // for swapchain extent
#include "renderer/Renderer.h"

#include <imgui.h>
#include <glm/glm.hpp>

namespace RDE {
    TestSceneLayer::TestSceneLayer(AssetManager *asset_manager, entt::registry &registry, RAL::Device *device, Renderer *renderer)
        : m_asset_manager(asset_manager), m_registry(registry), m_device(device), m_renderer(renderer) {}

    TestSceneLayer::~TestSceneLayer() {
        destroy_triangle_resources();
    }

    void TestSceneLayer::on_attach() {
        // Minimal: create GPU triangle + one entity instance
        create_triangle_resources();
        auto e = m_registry.create();
        m_registry.emplace<TransformLocal>(e); // default at origin
        m_registry.emplace<TriangleTag>(e);
    }

    void TestSceneLayer::on_detach() {
        destroy_triangle_resources();
    }

    void TestSceneLayer::on_update([[maybe_unused]] float delta_time) {}

    void TestSceneLayer::on_event([[maybe_unused]] Event &e) {}

    void TestSceneLayer::on_render(RAL::CommandBuffer *cmd) {
        if(!cmd) return;
        if(!m_trianglePipeline.is_valid() || !m_triangleVertexBuffer.is_valid()) return;
        if(auto *vkDev = dynamic_cast<VulkanDevice*>(m_device)) {
            auto extent = vkDev->get_swapchain().get_extent();
            RAL::Viewport vp{0.f,0.f,(float)extent.width,(float)extent.height,0.f,1.f};
            cmd->set_viewport(vp);
            RAL::Rect2D sc{0,0,extent.width,extent.height};
            cmd->set_scissor(sc);
        }
        cmd->bind_pipeline(m_trianglePipeline);
        // Bind camera descriptor set (set=0) if available
        if(m_renderer && m_renderer->get_camera_descriptor_set().is_valid()) {
            cmd->bind_descriptor_set(m_trianglePipeline, m_renderer->get_camera_descriptor_set(), 0);
        }
        cmd->bind_vertex_buffer(m_triangleVertexBuffer, 0);
        auto view = m_registry.view<TriangleTag, TransformLocal>();
        for(auto [ent, tx] : view.each()) {
            glm::mat4 model = TransformUtils::GetModelMatrix(tx);
            cmd->push_constants(m_trianglePipeline, RAL::ShaderStage::Vertex, 0, sizeof(glm::mat4), &model);
            cmd->draw(3,1,0,0);
        }
    }

    void TestSceneLayer::on_render_gui() {
        // Simple ImGui panel to add/remove triangles
        if(ImGui::Begin("Triangles")) {
            if(ImGui::Button("Add Triangle")) {
                auto e = m_registry.create();
                auto &t = m_registry.emplace<TransformLocal>(e);
                // Offset new triangle slightly
                t.translation.x = (float)(int)(m_registry.storage<entt::entity>().size() % 5) * 0.2f;
                m_registry.emplace<TriangleTag>(e);
            }
            int count = 0;
            auto view = m_registry.view<TriangleTag, TransformLocal>();
            for(auto [ent, tr] : view.each()) {
                ImGui::PushID((int)ent);
                if(ImGui::TreeNode("Triangle")) {
                    ImGui::DragFloat3("Translation", &tr.translation.x, 0.01f);
                    glm::vec3 euler = glm::eulerAngles(tr.orientation);
                    if(ImGui::DragFloat3("Rotation(rad)", &euler.x, 0.01f)) {
                        tr.orientation = glm::quat(euler);
                    }
                    ImGui::DragFloat3("Scale", &tr.scale.x, 0.01f, 0.01f, 10.f);
                    if(ImGui::Button("Delete")) { m_registry.destroy(ent); ImGui::TreePop(); ImGui::PopID(); break; }
                    ImGui::TreePop();
                }
                ImGui::PopID();
                ++count;
            }
            ImGui::Text("Count: %d", count);
        }
        ImGui::End();
    }

    void TestSceneLayer::create_triangle_resources() {
        if(!m_device) { RDE_CORE_WARN("TestSceneLayer: no device - cannot create triangle resources"); return; }
        if(m_trianglePipeline.is_valid()) return; // already created

        // 1. Create vertex buffer (pos(vec2) + color(vec3))
        struct Vertex { float px, py, pz; float r,g,b; }; // now 3D position
        Vertex vertices[3] = {
            { 0.0f, -0.5f, 0.0f, 1.f, 0.f, 0.f},
            { 0.5f,  0.5f, 0.0f, 0.f, 1.f, 0.f},
            {-0.5f,  0.5f, 0.0f, 0.f, 0.f, 1.f}
        };
        RAL::BufferDescription vbDesc{}; vbDesc.size = sizeof(vertices); vbDesc.usage = RAL::BufferUsage::VertexBuffer; vbDesc.memoryUsage = RAL::MemoryUsage::HostVisibleCoherent;
        m_triangleVertexBuffer = m_device->create_buffer(vbDesc);
        m_device->update_buffer_data(m_triangleVertexBuffer, vertices, sizeof(vertices), 0);

        // 2. Load precompiled SPIR-V shaders (assuming they exist at data/shaders/spirv/vert.spv & frag.spv)
        auto assetPath = get_asset_path();
        if(!assetPath) { RDE_CORE_ERROR("Asset path unavailable - triangle shaders missing"); return; }
        std::string vertSpv = (assetPath.value() / "shaders" / "spirv" / "Triangle.vert.spv").string();
        std::string fragSpv = (assetPath.value() / "shaders" / "spirv" / "Triangle.frag.spv").string();
        auto vertCode = FileIO::ReadFile(vertSpv);
        auto fragCode = FileIO::ReadFile(fragSpv);
        if(vertCode.empty() || fragCode.empty()) { RDE_CORE_ERROR("Failed to read triangle SPIR-V binaries"); return; }
        m_triangleVS = m_device->create_shader_module(vertCode, RAL::ShaderStage::Vertex);
        m_triangleFS = m_device->create_shader_module(fragCode, RAL::ShaderStage::Fragment);

        // 3. Create pipeline description (no descriptor sets, no push constants)
        RAL::PipelineDescription pDesc{};
        RAL::GraphicsShaderStages stages{}; stages.vertexShader = m_triangleVS; stages.fragmentShader = m_triangleFS; pDesc.stages = stages;
        // Include camera descriptor set layout if renderer has one
        if(m_renderer && m_renderer->get_camera_set_layout().is_valid()) {
            pDesc.descriptorSetLayouts.push_back(m_renderer->get_camera_set_layout());
        }
        // Add push constant range for model matrix
        RAL::PushConstantRange pc{}; pc.stages = RAL::ShaderStage::Vertex; pc.offset = 0; pc.size = sizeof(glm::mat4); pc.name = "Model"; pDesc.pushConstantRanges.push_back(pc);
        // Vertex layout
        RAL::VertexInputBinding binding{}; binding.binding = 0; binding.stride = sizeof(Vertex); pDesc.vertexBindings.push_back(binding);
        RAL::VertexInputAttribute attrPos{}; attrPos.location = 0; attrPos.binding = 0; attrPos.format = RAL::Format::R32G32B32_SFLOAT; attrPos.offset = 0; attrPos.name = "POSITION"; pDesc.vertexAttributes.push_back(attrPos);
        RAL::VertexInputAttribute attrColor{}; attrColor.location = 1; attrColor.binding = 0; attrColor.format = RAL::Format::R32G32B32_SFLOAT; attrColor.offset = sizeof(float)*3; attrColor.name = "COLOR"; pDesc.vertexAttributes.push_back(attrColor);
        // NOTE: Triangle shaders should be updated to consume position (vec3) & color (vec3) and a push constant mat4.
        pDesc.topology = RAL::PrimitiveTopology::TriangleList;
        pDesc.rasterizationState.cullMode = RAL::CullMode::None; // show both sides
        // Match current render pass depth attachment format (present even if we do not test depth)
        pDesc.depthAttachmentFormat = RAL::Format::D32_SFLOAT; // NEW to satisfy validation
        // Explicitly keep depth test disabled
        pDesc.depthStencilState.depthTestEnable = false;
        pDesc.depthStencilState.depthWriteEnable = false;
        // Create pipeline
        m_trianglePipeline = m_device->create_pipeline(pDesc);
        if(!m_trianglePipeline.is_valid()) {
            RDE_CORE_ERROR("Failed to create triangle pipeline");
        }
    }

    void TestSceneLayer::destroy_triangle_resources() {
        if(!m_device) return;
        if(m_trianglePipeline.is_valid()) { m_device->destroy_pipeline(m_trianglePipeline); m_trianglePipeline = RAL::PipelineHandle::INVALID(); }
        if(m_triangleVS.is_valid()) { m_device->destroy_shader(m_triangleVS); m_triangleVS = RAL::ShaderHandle::INVALID(); }
        if(m_triangleFS.is_valid()) { m_device->destroy_shader(m_triangleFS); m_triangleFS = RAL::ShaderHandle::INVALID(); }
        if(m_triangleVertexBuffer.is_valid()) { m_device->destroy_buffer(m_triangleVertexBuffer); m_triangleVertexBuffer = RAL::BufferHandle::INVALID(); }
    }
}
