#include "renderer/PipelineCache.h"
#include "renderer/ShaderReflector.h" // Include our new utility
#include "assets/AssetComponentTypes.h" // Include our new utility
#include "ral/Common.h"         // Include our new utility
#include "ral/EnumUtils.h"          // Include our new utility
#include "core/FileIOUtils.h"          // Include our new utility
#include "core/Paths.h"          // Include our new utility
#include "core/Log.h"          // For logging

#include <functional>

// And the hash specialization
namespace std {
    template<>
    struct hash<RDE::PipelineCache::PipelineVariantKey> {
        size_t operator()(const RDE::PipelineCache::PipelineVariantKey &key) const {
            size_t h1 = std::hash<entt::entity>()(key.shader_def_entity);
            size_t h2 = std::hash<RDE::ShaderFeatureMask>()(key.mask);
            return h1 ^ (h2 << 1);
        }
    };
}

namespace RDE {
    // Note: The rest of the PipelineCache implementation is in the header in your example.
    // This is the full implementation of the build function.

    RAL::PipelineHandle PipelineCache::buildAndCachePipeline(AssetID shader_def_id, ShaderFeatureMask mask,
                                                             const PipelineVariantKey &key) {
        // A. Get the recipe from the AssetDatabase
        auto *def = m_asset_manager.get_database().try_get<AssetCpuShaderDefinition>(shader_def_id);
        if (!def) {
            RDE_CORE_ERROR("PipelineCache: Could not find shader definition for asset!");
            return RAL::PipelineHandle::INVALID();
        }

        RAL::PipelineDescription pso_info = {};
        std::map<RAL::ShaderStage, const std::vector<char> *> shader_stages_map;
        std::vector<char> vert_bytecode, frag_bytecode, geom_bytecode, tess_cntrl_bytecode, tess_eval_bytecode,
                compute_bytecode, task_bytecode, mesh_bytecode; // etc.

        // B. Load required shader bytecodes
        auto load_shader = [&](RAL::ShaderStage stage, std::vector<char> &out_bytecode) {
            auto it = def->base_spirv_paths.find(stage);
            if (it != def->base_spirv_paths.end()) {
                const std::string path = it->second + "." + std::to_string(mask) + ".spv";
                out_bytecode = FileIO::ReadFile(get_spirv_path().value() / path);
                if (!out_bytecode.empty()) {
                    shader_stages_map[stage] = &out_bytecode;
                }
            }
        };

        load_shader(RAL::ShaderStage::Vertex, vert_bytecode);
        load_shader(RAL::ShaderStage::Fragment, frag_bytecode);
        load_shader(RAL::ShaderStage::Geometry, geom_bytecode);
        load_shader(RAL::ShaderStage::TessellationControl, tess_cntrl_bytecode);
        load_shader(RAL::ShaderStage::TessellationEvaluation, tess_eval_bytecode);
        load_shader(RAL::ShaderStage::Compute, compute_bytecode);
        load_shader(RAL::ShaderStage::Task, task_bytecode);
        load_shader(RAL::ShaderStage::Mesh, mesh_bytecode);
        // ... load other stages if needed ...

        if (shader_stages_map.empty()) {
            RDE_CORE_ERROR("PipelineCache: No shader stages loaded for '{}'!", def->name);
            return RAL::PipelineHandle::INVALID();
        }

        // C. REFLECT the loaded bytecode to get the pipeline layout
        ReflectedLayout reflected = ShaderReflector::reflect(shader_stages_map);

        // D. CREATE the descriptor set layouts and populate the PSO description
        for (const auto &[set_index, layout_desc]: reflected.setLayouts) {
            pso_info.descriptorSetLayouts.push_back(m_device.create_descriptor_set_layout(layout_desc));
        }
        pso_info.pushConstantRanges = reflected.pushConstantRanges;

        // E. CREATE the shader modules
        if (shader_stages_map.count(RAL::ShaderStage::Vertex)) {
            pso_info.vertexShader = m_device.create_shader_module(vert_bytecode, RAL::ShaderStage::Vertex);
        }
        if (shader_stages_map.count(RAL::ShaderStage::Fragment)) {
            pso_info.fragmentShader = m_device.create_shader_module(frag_bytecode, RAL::ShaderStage::Fragment);
        }
        if (shader_stages_map.count(RAL::ShaderStage::Geometry)) {
            pso_info.geometryShader = m_device.create_shader_module(geom_bytecode, RAL::ShaderStage::Geometry);
        }
        if (shader_stages_map.count(RAL::ShaderStage::TessellationControl)) {
            pso_info.tessControlShader = m_device.create_shader_module(tess_cntrl_bytecode, RAL::ShaderStage::TessellationControl);
        }
        if (shader_stages_map.count(RAL::ShaderStage::TessellationEvaluation)) {
            pso_info.tessEvalShader = m_device.create_shader_module(tess_eval_bytecode, RAL::ShaderStage::TessellationEvaluation);
        }
        if (shader_stages_map.count(RAL::ShaderStage::Compute)) {
            pso_info.computeShader = m_device.create_shader_module(compute_bytecode, RAL::ShaderStage::Compute);
        }
        if (shader_stages_map.count(RAL::ShaderStage::Task)) {
            pso_info.taskShader = m_device.create_shader_module(task_bytecode, RAL::ShaderStage::Task);
        }
        if (shader_stages_map.count(RAL::ShaderStage::Mesh)) {
            pso_info.meshShader = m_device.create_shader_module(mesh_bytecode, RAL::ShaderStage::Mesh);
        }

        // F. APPLY fixed state and vertex layout from the definition
        pso_info.rasterizationState.cullMode = def->cull_mode;
        pso_info.depthStencilState.depthTestEnable = def->depth_test;
        pso_info.depthStencilState.depthWriteEnable = def->depth_write;
        // ... copy all other state from *def to pso_info ...

        uint32_t current_offset = 0;
        for (size_t i = 0; i < def->vertex_layout.size(); ++i) {
            const auto &attr = def->vertex_layout[i];
            pso_info.vertexAttributes.push_back({
                static_cast<uint32_t>(i), // location
                0, // binding
                attr.format,
                current_offset
            });
            current_offset += get_size_of_format(attr.format);
        }
        if (current_offset > 0) {
            pso_info.vertexBindings.push_back({0, current_offset});
        }

        // G. CREATE the final GPU pipeline object via the RAL
        RAL::PipelineHandle pipeline_handle = m_device.create_pipeline(pso_info);
        m_cache[key] = pipeline_handle;

        RDE_CORE_INFO("PipelineCache: Compiled and cached pipeline for '{}' with mask {}.", def->name, mask);
        return pipeline_handle;
    }
}
