// file: renderer/PipelineCache.cpp

#include "renderer/PipelineCache.h"
#include "assets/AssetComponentTypes.h"
#include "assets/AssetManager.h"
#include "core/FileIOUtils.h" // Assuming this is still needed for loading SPIR-V
#include "core/Log.h"

namespace RDE {

    namespace { // Anonymous namespace for local helpers
        RAL::ShaderStage path_to_shader_stage(const std::string& path) {
            std::string ext = std::filesystem::path(path).extension().string();
            if (ext == ".vert") return RAL::ShaderStage::Vertex;
            if (ext == ".frag") return RAL::ShaderStage::Fragment;
            if (ext == ".comp") return RAL::ShaderStage::Compute;
            // Add more as needed
            return RAL::ShaderStage::None;
        }
    }

    // Constructor and Destructor remain largely the same.
    PipelineCache::PipelineCache(AssetManager &asset_manager, RAL::Device &device)
            : m_asset_manager(asset_manager), m_device(device) {}

    PipelineCache::~PipelineCache() {
        // ... destructor is fine, it cleans up RAL resources ...
        for (auto &[key, cached_pipeline]: m_cache) {
            // Clean up shader modules
            for (auto &shader: cached_pipeline.shaderModules) {
                m_device.destroy_shader(shader);
            }
            // Clean up descriptor set layouts
            for (auto &layout: cached_pipeline.setLayouts) {
                m_device.destroy_descriptor_set_layout(layout);
            }
            // The pipeline itself will be cleaned up by the RAL device.
        }
    }

    RAL::PipelineHandle PipelineCache::getPipeline(const AssetID &shader_def_id, ShaderFeatureMask feature_mask) {
        PipelineVariantKey key = {shader_def_id->entity_id, feature_mask};
        if (auto it = m_cache.find(key); it != m_cache.end()) {
            return it->second.pipeline;
        }
        return buildAndCachePipeline(shader_def_id, feature_mask, key);
    }

    // --- The COMPLETELY REFACTORED build function ---
    RAL::PipelineHandle PipelineCache::buildAndCachePipeline(RDE::AssetID shader_def_id, RDE::ShaderFeatureMask mask,
                                                             const RDE::PipelineVariantKey &key) {
        // 1. Get the contract (the shader definition) from the AssetDatabase.
        auto *shaderDef = m_asset_manager.get_database().try_get<AssetShaderDef>(shader_def_id);
        if (!shaderDef) {
            RDE_CORE_ERROR("PipelineCache: Could not find AssetShaderDef component for the provided asset ID!");
            return RAL::PipelineHandle::INVALID();
        }

        CachedPipeline new_cached_pipeline; // Our bundle for owning created RAL resources.

        // 2. CREATE LAYOUT FROM THE CONTRACT (NO REFLECTION!)
        // The shaderdef now explicitly tells us the layout.
        for (const auto &setLayoutDesc: shaderDef->descriptorSetLayouts) {
            std::vector<RAL::DescriptorSetLayoutBinding> ralBindings;
            for (const auto &binding: setLayoutDesc.bindings) {
                ralBindings.emplace_back(binding);
            }
            RAL::DescriptorSetLayoutDescription ralDesc;
            ralDesc.bindings = ralBindings;
            ralDesc.set = setLayoutDesc.set; // Assuming set is a field in the binding struct
            auto handle = m_device.create_descriptor_set_layout(ralDesc);
            new_cached_pipeline.setLayouts.push_back(handle);
        }

        // Convert push constant ranges from our asset struct to the RAL struct
        std::vector<RAL::PushConstantRange> ralPushConstantRanges;
        for (const auto &pcRange: shaderDef->pushConstantRanges) {
            ralPushConstantRanges.emplace_back(pcRange);
        }

        // 3. LOAD and CREATE Shader Modules for this specific permutation
        // The dependency list in the shaderDef gives us the base paths.
        // We append the permutation mask to get the correct file.
        const auto &spirvDeps = shaderDef->dependencies.spirv_dependencies;
        for (const auto &baseSpirvPath: spirvDeps) {
            std::string permutationPath = baseSpirvPath + "." + std::to_string(mask) + ".spv";

            // Assume the AssetManager can load raw binary blobs, or use FileIO for now.
            auto bytecode = FileIO::ReadFile(permutationPath);
            if (bytecode.empty()) {
                RDE_CORE_ERROR("PipelineCache: Failed to load SPIR-V file: {}", permutationPath);
                continue; // Or handle error more gracefully
            }

            // Determine the stage from the file extension (e.g., .vert, .frag)
            RAL::ShaderStage stage = path_to_shader_stage(baseSpirvPath); // You need this helper
            auto handle = m_device.create_shader_module(bytecode, stage);
            new_cached_pipeline.shaderModules.push_back(handle);
        }

        if (new_cached_pipeline.shaderModules.empty()) {
            RDE_CORE_ERROR("PipelineCache: No shader modules were loaded for '{}'.", shaderDef->name);
            // Cleanup layouts we just created
            for (auto h: new_cached_pipeline.setLayouts) m_device.destroy_descriptor_set_layout(h);
            return RAL::PipelineHandle::INVALID();
        }

        // 4. BUILD the final PipelineDescription
        RAL::PipelineDescription psoDesc;
        psoDesc.descriptorSetLayouts = new_cached_pipeline.setLayouts;
        psoDesc.pushConstantRanges = ralPushConstantRanges;

        // Find the appropriate shader handles from the ones we just created.
        bool is_graphics_pipeline = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Vertex).is_valid() &&
                            find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Fragment).is_valid();
        bool is_mesh_pipeline = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Task).is_valid() &&
                                find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Mesh).is_valid();
        bool is_compute_pipeline = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Compute).is_valid();
        if(is_graphics_pipeline){
            RAL::GraphicsShaderStages graphicsStages;
            graphicsStages.vertexShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Vertex);
            graphicsStages.fragmentShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Fragment);
            graphicsStages.geometryShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Geometry);
            graphicsStages.tessControlShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::TessellationControl);
            graphicsStages.tessEvalShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::TessellationEvaluation);
            psoDesc.stages = graphicsStages; // Set the graphics stages
        } else if(is_compute_pipeline){
            RAL::ComputeShaderStages computeStages;
            computeStages.computeShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Compute);
            psoDesc.stages = computeStages; // Set the compute stages
        }else if(is_mesh_pipeline){
            RAL::MeshShaderStages meshStages;
            meshStages.taskShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Task);
            meshStages.meshShader = find_shader_handle(new_cached_pipeline.shaderModules, RAL::ShaderStage::Mesh);
            psoDesc.stages = meshStages; // Set the mesh stages
        }



        // Check if this is a compute pipeline instead
        if (!is_compute_pipeline) {
            // --- GRAPHICS PIPELINE SETUP ---
            // Apply vertex layout directly from the contract
            uint32_t current_offset = 0;
            for (const auto &cond_attr: shaderDef->vertexAttributes) {
                RAL::VertexInputAttribute attr;
                attr.location = cond_attr.location;
                attr.format = cond_attr.format; // Assuming this is a valid RAL format
                attr.offset = current_offset;
                attr.name = cond_attr.name; // Optional, if you want to keep names
                psoDesc.vertexAttributes.emplace_back(attr);
                current_offset += get_size_of_format(attr.format); // You need this helper
            }
            if (current_offset > 0) {
                psoDesc.vertexBindings.push_back({0, current_offset});
            }
            // REMOVED: forcing a depth attachment format here caused validation issues when the
            // pipeline did not actually require depth. The renderer will inject the correct
            // depthAttachmentFormat later if a depth buffer is present for the frame.
            // psoDesc.depthAttachmentFormat = RAL::Format::D32_SFLOAT;
        }

        // 5. CREATE and CACHE the pipeline
        new_cached_pipeline.pipeline = m_device.create_pipeline(psoDesc);

        if (!new_cached_pipeline.pipeline.is_valid()) {
            RDE_CORE_ERROR("PipelineCache: Device failed to create pipeline for '{}' mask {}.", shaderDef->name, mask);
            // Manually clean up this failed attempt's resources
            for (auto h: new_cached_pipeline.shaderModules) m_device.destroy_shader(h);
            for (auto h: new_cached_pipeline.setLayouts) m_device.destroy_descriptor_set_layout(h);
            return RAL::PipelineHandle::INVALID();
        }

        RDE_CORE_INFO("PipelineCache: Compiled and cached pipeline for '{}' mask {}.", shaderDef->name, mask);
        auto [it, success] = m_cache.emplace(key, std::move(new_cached_pipeline));
        return it->second.pipeline;
    }

    // Helper function to find a specific shader handle from a list
    RAL::ShaderHandle
    PipelineCache::find_shader_handle(const std::vector<RAL::ShaderHandle> &handles, RAL::ShaderStage stage) const {
        for (auto handle: handles) {
            if (!m_device.get_resources_database().is_valid(handle)) continue;
            // Shaders created via create_shader_module store a RAL::ShaderStage component directly.
            const auto &storedStage = m_device.get_resources_database().get<RAL::ShaderStage>(handle);
            if (storedStage == stage) {
                return handle;
            }
        }
        return RAL::ShaderHandle::INVALID();
    }
}
