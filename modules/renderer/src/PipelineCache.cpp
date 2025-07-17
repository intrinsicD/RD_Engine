#include "renderer/PipelineCache.h"
#include "renderer/ShaderReflector.h" // Include our new utility
#include "assets/AssetComponentTypes.h" // Include our new utility
#include "ral/Common.h"         // Include our new utility
#include "ral/EnumUtils.h"          // Include our new utility
#include "core/FileIOUtils.h"          // Include our new utility
#include "core/Paths.h"          // Include our new utility
#include "core/Log.h"          // For logging


namespace RDE {
    // Note: The rest of the PipelineCache implementation is in the header in your example.
    // This is the full implementation of the build function.

    PipelineCache::PipelineCache(AssetManager &asset_manager, RAL::Device &device)
            : m_asset_manager(asset_manager), m_device(device), m_cache() {
    }

    PipelineCache::~PipelineCache() {
        RDE_CORE_INFO("PipelineCache shutting down. Cleaning up {} cached pipeline variants.", m_cache.size());
        for (const auto& [key, cached_pipeline] : m_cache) {
            // Destroy resources in reverse order of creation
            m_device.destroy_pipeline(cached_pipeline.pipeline);
            for (auto handle : cached_pipeline.shaderModules) {
                m_device.destroy_shader(handle);
            }
            for (auto handle : cached_pipeline.setLayouts) {
                m_device.destroy_descriptor_set_layout(handle);
            }
        }
        m_cache.clear();
    }

    // The main interface for the renderer.
    RAL::PipelineHandle PipelineCache::getPipeline(const AssetID &shader_def_id, ShaderFeatureMask feature_mask) {
        // 1. Create a unique key for this specific pipeline variant.
        PipelineVariantKey key = {shader_def_id->entity_id, feature_mask};
        if (auto it = m_cache.find(key); it != m_cache.end()) {
            return it->second.pipeline;
        }

        // 2. Cache miss: We need to build it.
        return buildAndCachePipeline(shader_def_id, feature_mask, key);
    }

    RAL::PipelineHandle PipelineCache::buildAndCachePipeline(AssetID shader_def_id, ShaderFeatureMask mask,
                                                             const PipelineVariantKey &key) {
        // A. Get the recipe from the AssetDatabase
        auto* def = m_asset_manager.get_database().try_get<AssetCpuShaderDefinition>(shader_def_id);
        if (!def) {
            RDE_CORE_ERROR("PipelineCache: Could not find shader definition asset!");
            return RAL::PipelineHandle::INVALID();
        }

        // B. Load required shader bytecodes
        std::map<RAL::ShaderStage, std::vector<char>> loaded_bytecodes;
        std::map<RAL::ShaderStage, const std::vector<char>*> shader_stages_map;

        auto load_shader = [&](RAL::ShaderStage stage) {
            auto it = def->base_spirv_paths.find(stage);
            if (it != def->base_spirv_paths.end()) {
                const std::string path = it->second + "." + std::to_string(mask) + ".spv";
                loaded_bytecodes[stage] = FileIO::ReadFile(get_spirv_path().value() / path);
                if (!loaded_bytecodes[stage].empty()) {
                    shader_stages_map[stage] = &loaded_bytecodes[stage];
                } else {
                    RDE_CORE_ERROR("PipelineCache: Failed to load SPIR-V file: {}", path);
                }
            }
        };

        for(const auto& [stage, path] : def->base_spirv_paths) {
            load_shader(stage);
        }

        if (shader_stages_map.empty()) {
            RDE_CORE_ERROR("PipelineCache: No shader stages loaded for '{}'!", def->name);
            return RAL::PipelineHandle::INVALID();
        }

        // C. REFLECT the loaded bytecode to get the pipeline layout
        ReflectedLayout reflected = ShaderReflector::reflect(shader_stages_map);

        // This is our ownership bundle for all created resources.
        CachedPipeline new_cached_pipeline;

        // D. CREATE the descriptor set layouts and store them in our ownership bundle
        for (const auto& [set_index, layout_desc] : reflected.setLayouts) {
            auto handle = m_device.create_descriptor_set_layout(layout_desc);
            new_cached_pipeline.setLayouts.push_back(handle);
        }

        // E. CREATE the shader modules and store them in our ownership bundle
        for(const auto& [stage, bytecode_ptr] : shader_stages_map) {
            auto handle = m_device.create_shader_module(*bytecode_ptr, stage);
            new_cached_pipeline.shaderModules.push_back(handle);
        }

        // F. BUILD the appropriate pipeline description (Graphics vs Compute)
        if (shader_stages_map.count(RAL::ShaderStage::Compute)) {
            // --- COMPUTE PIPELINE PATH ---
            RAL::PipelineDescription compute_pso_info;
            compute_pso_info.computeShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Compute);
            compute_pso_info.descriptorSetLayouts = new_cached_pipeline.setLayouts; // Copy handles
            compute_pso_info.pushConstantRanges = reflected.pushConstantRanges;

            new_cached_pipeline.pipeline = m_device.create_pipeline(compute_pso_info);

        } else {
            // --- GRAPHICS PIPELINE PATH ---
            RAL::PipelineDescription graphics_pso_info;

            // F-1: Assign layouts and push constants
            graphics_pso_info.descriptorSetLayouts = new_cached_pipeline.setLayouts; // Copy handles
            graphics_pso_info.pushConstantRanges = reflected.pushConstantRanges;

            // F-2: Assign shader modules
            graphics_pso_info.vertexShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Vertex);
            graphics_pso_info.fragmentShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Fragment);
            // ... assign other graphics stages (geometry, mesh, etc.) ...

            // F-3: Apply fixed state and vertex layout from the definition
            graphics_pso_info.rasterizationState.cullMode = def->cull_mode;
            graphics_pso_info.depthStencilState.depthTestEnable = def->depth_test;
            graphics_pso_info.depthStencilState.depthWriteEnable = def->depth_write;
            // ... copy all other state from *def to graphics_pso_info ...

            uint32_t current_offset = 0;
            for (size_t i = 0; i < def->vertex_layout.size(); ++i) {
                const auto& attr = def->vertex_layout[i];
                graphics_pso_info.vertexAttributes.push_back({ static_cast<uint32_t>(i), 0, attr.format, current_offset });
                current_offset += get_size_of_format(attr.format);
            }
            if (current_offset > 0) {
                graphics_pso_info.vertexBindings.push_back({0, current_offset});
            }

            new_cached_pipeline.pipeline = m_device.create_pipeline(graphics_pso_info);
        }

        // G. VALIDATE and CACHE the final result
        if (!new_cached_pipeline.pipeline.is_valid()) {
            RDE_CORE_ERROR("PipelineCache: Device failed to create pipeline for '{}' with mask {}.", def->name, mask);
            // Manually clean up the resources we just created for this failed attempt
            for (auto handle : new_cached_pipeline.shaderModules) m_device.destroy_shader(handle);
            for (auto handle : new_cached_pipeline.setLayouts) m_device.destroy_descriptor_set_layout(handle);
            return RAL::PipelineHandle::INVALID();
        }

        RDE_CORE_INFO("PipelineCache: Compiled and cached pipeline for '{}' with mask {}.", def->name, mask);

        auto [it, success] = m_cache.emplace(key, std::move(new_cached_pipeline));
        return it->second.pipeline;
    }

    // Helper function to find a specific shader handle from a list
    RAL::ShaderHandle PipelineCache::find_shader_handle(const std::vector<RAL::ShaderHandle>& handles, RAL::ShaderStage stage) const {
        for(auto handle : handles) {
            // This assumes your RAL::Device can query the stage of a ShaderHandle.
            // If not, you'll need to store the stage alongside the handle when creating them.
            if (m_device.get_resources_database().get<RAL::ShaderDescription>(handle).stage == stage) {
                return handle;
            }
        }
        return RAL::ShaderHandle::INVALID();
    }
}
