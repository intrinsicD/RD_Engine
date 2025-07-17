// core/EnumUtils.h or similar
#pragma once

#include "ral/Resources.h"

#include <string>
#include <stdexcept>

namespace RDE {
    inline RAL::ShaderStage string_to_shader_stage(const std::string &s) {
        if (s == "vertex") return RAL::ShaderStage::Vertex;
        if (s == "fragment") return RAL::ShaderStage::Fragment;
        if (s == "compute") return RAL::ShaderStage::Compute;
        if (s == "geometry") return RAL::ShaderStage::Geometry;
        if (s == "tess_control") return RAL::ShaderStage::TessellationControl;
        if (s == "tess_evaluation") return RAL::ShaderStage::TessellationEvaluation;
        if (s == "task") return RAL::ShaderStage::Task;
        if (s == "mesh") return RAL::ShaderStage::Mesh;
        // Add other stages as needed...
        throw std::runtime_error("Unknown shader stage: " + s);
    }

    inline RAL::Format string_to_ral_format(const std::string &s) {
        if (s == "R32G32B32A32_SFLOAT") return RAL::Format::R32G32B32A32_SFLOAT;
        if (s == "R32G32B32_SFLOAT") return RAL::Format::R32G32B32_SFLOAT;
        if (s == "R32G32_SFLOAT") return RAL::Format::R32G32_SFLOAT;
        if (s == "R32_SFLOAT") return RAL::Format::R32_SFLOAT;
        if (s == "R8G8B8A8_UNORM") return RAL::Format::R8G8B8A8_UNORM;
        // Add all other formats you intend to support in your vertex layouts...
        throw std::runtime_error("Unknown RAL format: " + s);
    }

    // While we're at it, let's add a helper for the pipeline state.
    inline RAL::CullMode string_to_cull_mode(const std::string &s) {
        if (s == "None") return RAL::CullMode::None;
        if (s == "Front") return RAL::CullMode::Front;
        if (s == "Back") return RAL::CullMode::Back;
        return RAL::CullMode::Back; // Default
    }
} // namespace RDE
