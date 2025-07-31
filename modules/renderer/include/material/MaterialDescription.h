#pragma once

#include "core/Properties.h"
#include "ral/Resources.h"
#include "assets/AssetHandle.h"

namespace RDE{
    // MaterialDesctiption is a struct that describes a material, including its name, parameters, and textures.
    // Each material instance will have a unique MaterialID that can be used to reference it in the rendering system.
    // Each material is separated into a MaterialDescription and a GpuMaterial.
    // MaterialDescription contains the high-level description of the material, while GpuMaterial contains the GPU-specific data needed for rendering.

    struct MaterialDescription{
        std::string name;
        AssetID pipeline; // Tag to identify the pipeline this material uses
        PropertyContainer parameters; //uniforms and rendering state parameters (PropertyContainer is a sophisticated soa data structure, see pmp library)
        std::unordered_map<std::string, AssetID> textures;
    };

    struct GpuMaterial{
        RAL::PipelineHandle pipeline_id; // Handle to the pipeline this material uses
        RAL::DescriptorSetHandle descriptor_set_id; // Handle to the descriptor set for this material
        RAL::BufferHandle ubo; // Handle to the uniform buffer object (UBO) for this material
        std::unordered_map<std::string, RAL::TextureHandle> textures; // Texture handles used by this material
    };
}