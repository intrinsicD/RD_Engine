#pragma once

#include "assets/AssetHandle.h"
#include "core/Properties.h"
#include "ral/Resources.h"

namespace RDE{
    struct MaterialComponent{
        AssetID material_asset_id; // Reference to the material asset

        PropertyContainer parameters; //uniforms and other parameters for the material

        std::unordered_map<std::string, AssetID> texture_bindings;

        bool is_valid() const {
            return material_asset_id ? material_asset_id->is_valid() : false;
        }
    };

    struct RenderableMaterial {
        // The pipeline is determined by the material's shader and pipeline state.
        RAL::PipelineHandle pipeline;

        // Each instance gets its OWN descriptor set and UBO because its parameters can be unique.
        RAL::DescriptorSetHandle descriptor_set;
        RAL::BufferHandle uniform_buffer;
    };
}