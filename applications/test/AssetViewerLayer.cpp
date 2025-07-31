#include "AssetViewerLayer.h"
#include "assets/AssetComponentTypes.h"
#include "material/MaterialDescription.h"
#include "ral/EnumUtils.h"


#include <imgui.h>

namespace RDE {
    void AssetViewerLayer::on_render_gui() {
        auto &asset_registry = m_asset_database->get_registry();
        auto all_assets = asset_registry.view<entt::entity>();
        // Show a tree of all assets
        ImGui::Begin("Asset Viewer");
        for (const auto &asset: all_assets) {
            // make a new node in the tree
            ImGui::PushID(entt::to_integral(asset));

            if (ImGui::TreeNode(asset_registry.get<AssetName>(asset).name.c_str())) {
                // Show asset details

                auto &asset_name = asset_registry.get<AssetName>(asset);
                ImGui::Text("Name: %s", asset_name.name.c_str());
                if (asset_registry.all_of<AssetFilepath>(asset)) {
                    auto &asset_filepath = asset_registry.get<AssetFilepath>(asset);
                    ImGui::Text("Path: %s", asset_filepath.path.c_str());
                }
                if (asset_registry.all_of<AssetTextSource>(asset)) {
                    auto &asset_text_source = asset_registry.get<AssetTextSource>(asset);
                    ImGui::Text("Text Source: %s", asset_text_source.text.c_str());
                }
                if (asset_registry.all_of<AssetShaderModule>(asset)) {
                    auto &shader_module = asset_registry.get<AssetShaderModule>(asset);
                    ImGui::Text("RAL::ShaderHandle: %u", entt::to_integral(shader_module.module_handle.index));
                    ImGui::Text("RAL::ShaderStage: %s", shader_stage_to_string(shader_module.stage).c_str());
                }
                if (asset_registry.all_of<AssetPipeline>(asset)) {
                    auto &pipeline = asset_registry.get<AssetPipeline>(asset);
                    ImGui::Text("RAL::PipelineHandle: %u", entt::to_integral(pipeline.pipeline_handle.index));
                    ImGui::Text("Shaders: %zu", pipeline.shaders.size());
                    for (const auto &shader_id: pipeline.shaders) {
                        ImGui::Text(" - Shader ID: %u", entt::to_integral(shader_id->entity_id));
                    }
                }
                if (asset_registry.all_of<AssetPipelineDescription>(asset)) {
                    auto &pipeline_desc = asset_registry.get<AssetPipelineDescription>(asset);
                    ImGui::Text("Cull Mode: %s", cull_mode_to_string(pipeline_desc.cullMode).c_str());
                    ImGui::Text("Polygon Mode: %s", polygon_mode_to_string(pipeline_desc.polygonMode).c_str());
                    ImGui::Text("Depth Test: %s", pipeline_desc.depthTest ? "Enabled" : "Disabled");
                    ImGui::Text("Depth Write: %s", pipeline_desc.depthWrite ? "Enabled" : "Disabled");
                }
                if (asset_registry.all_of<MaterialDescription>(asset)) {
                    auto &material = asset_registry.get<MaterialDescription>(asset);
                    ImGui::Text("Material Name: %s", material.name.c_str());
                    if(material.pipeline){
                        ImGui::Text("Pipeline Asset: %u, uri: %s", entt::to_integral(material.pipeline->entity_id),
                                    material.pipeline->uri.c_str());
                    }
                    ImGui::Text("Parameters: ");
                    for (const auto &param: material.parameters.properties()) {
                        ImGui::Text(" - %s", param.c_str());
                    }
                    ImGui::Text("Texture Bindings: ");
                    for (const auto &texture_binding: material.textures) {
                        auto asset_id = texture_binding.second ? texture_binding.second->entity_id : entt::null;
                        ImGui::Text(" - %s: %u, uri: %s", texture_binding.first.c_str(),
                                    entt::to_integral(asset_id), texture_binding.second->uri.c_str());
                    }
                }
                if (asset_registry.all_of<AssetGpuGeometry>(asset)) {
                    auto &gpu_geometry = asset_registry.get<AssetGpuGeometry>(asset);
                    for (const auto &buffer_pair: gpu_geometry.buffers) {
                        ImGui::Text("Buffer: %s, Handle: %u", buffer_pair.first.c_str(),
                                    entt::to_integral(buffer_pair.second.index));
                    }
                    ImGui::Text("GPU Geometry: %zu subviews", gpu_geometry.subviews.size());
                    for (const auto &subview: gpu_geometry.subviews) {
                        ImGui::Text(" - Subview: %s", subview.name.c_str());
                    }
                }

                if (asset_registry.all_of<AssetGpuTexture>(asset)) {
                    auto &gpu_texture = asset_registry.get<AssetGpuTexture>(asset);
                    ImGui::Text("GPU Texture: %u", entt::to_integral(gpu_texture.texture.index));
                    ImGui::Text("Width: %d, Height: %d, Channels: %d", gpu_texture.width, gpu_texture.height,
                                gpu_texture.channels);
                }

                if (asset_registry.all_of<AssetShaderDef>(asset)) {
                    auto &shader_def = asset_registry.get<AssetShaderDef>(asset);
                    ImGui::Text("Shader Definition: %s", shader_def.name.c_str());
                    ImGui::Text("Dependencies:");
                    for (const auto &dep: shader_def.dependencies) {
                        ImGui::Text(" - %s:", dep.first.c_str());
                        for (const auto &dep_child: dep.second) {
                            ImGui::Text("   - %s", dep_child.c_str());
                        }
                    }
                    ImGui::Text("Features: ");
                    for (const auto &feature: shader_def.features) {
                        ImGui::Text(" - %s", feature.c_str());
                    }
                    ImGui::Text("Vertex Attributes: %zu", shader_def.vertexAttributes.size());
                    for (const auto &attr: shader_def.vertexAttributes) {
                        ImGui::Text(" - Attribute: %s, Binding: %u, Format: %s", attr.name.c_str(), attr.binding,
                                    ral_format_to_string(attr.format).c_str());
                    }
                }

                if (asset_registry.all_of<AssetCpuGeometry>(asset)) {
                    auto &cpu_geometry = asset_registry.get<AssetCpuGeometry>(asset);
                    ImGui::Text("CPU Geometry: %zu vertices", cpu_geometry.getVertexCount());
                    ImGui::Text("Subviews: %zu", cpu_geometry.subviews.size());
                    for (const auto &subview: cpu_geometry.subviews) {
                        ImGui::Text(" - Subview: %s, Index Offset: %u, Index Count: %u", subview.name.c_str(),
                                    subview.index_offset, subview.index_count);
                    }
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::End();
    }
}