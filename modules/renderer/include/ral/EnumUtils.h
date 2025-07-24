// core/EnumUtils.h or similar
#pragma once

#include "ral/Resources.h"

#include <string>
#include <stdexcept>

namespace RDE {
    inline std::string trim_string(const std::string& s) {
        size_t first = s.find_first_not_of(" \t\n\r");
        if (std::string::npos == first) {
            return s;
        }
        size_t last = s.find_last_not_of(" \t\n\r");
        return s.substr(first, (last - first + 1));
    }

    inline RAL::ShaderStage string_to_shader_stage(const std::string &s) {
        std::string lower_s = s;
        std::transform(lower_s.begin(), lower_s.end(), lower_s.begin(), ::tolower);

        if (lower_s == "vertex") return RAL::ShaderStage::Vertex;
        if (lower_s == "fragment") return RAL::ShaderStage::Fragment;
        if (lower_s == "compute") return RAL::ShaderStage::Compute;
        if (lower_s == "geometry") return RAL::ShaderStage::Geometry;
        if (lower_s == "tess_control") return RAL::ShaderStage::TessellationControl;
        if (lower_s == "tess_evaluation") return RAL::ShaderStage::TessellationEvaluation;
        if (lower_s == "task") return RAL::ShaderStage::Task;
        if (lower_s == "mesh") return RAL::ShaderStage::Mesh;
        // Add other stages as needed...
        throw std::runtime_error("Unknown shader stage: " + s);
    }

    inline RAL::ShaderStage string_to_shader_stages_mask(const std::string& stages_str) {
        RAL::ShaderStage mask = RAL::ShaderStage::None;
        std::stringstream ss(stages_str);
        std::string stage_token;

        while (std::getline(ss, stage_token, ',')) {
            try {
                std::string trimmed_token = trim_string(stage_token);
                mask |= string_to_shader_stage(trimmed_token);
            } catch (const std::runtime_error& e) {
                // Re-throw with more context
                throw std::runtime_error("Failed to parse stage token '" + stage_token + "' from full string '" + stages_str + "'. Reason: " + e.what());
            }
        }

        if (mask == RAL::ShaderStage::None) {
            throw std::runtime_error("Parsed shader stages string '" + stages_str + "' but resulted in an empty mask.");
        }

        return mask;
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

    inline RAL::PolygonMode string_to_polygon_mode(const std::string &s) {
        if (s == "Fill") return RAL::PolygonMode::Fill;
        if (s == "Line") return RAL::PolygonMode::Line;
        if (s == "Point") return RAL::PolygonMode::Point;
        throw std::runtime_error("Unknown polygon mode: " + s);
    }

    inline RAL::DescriptorType string_to_descriptor_type(const std::string &s) {
        if (s == "UniformBuffer") return RAL::DescriptorType::UniformBuffer;
        if (s == "StorageBuffer") return RAL::DescriptorType::StorageBuffer;
        if (s == "SampledImage") return RAL::DescriptorType::SampledImage;
        if (s == "StorageImage") return RAL::DescriptorType::StorageImage;
        if (s == "CombinedImageSampler") return RAL::DescriptorType::CombinedImageSampler;
        if (s == "Sampler") return RAL::DescriptorType::Sampler;
        // Add other descriptor types as needed...
        throw std::runtime_error("Unknown descriptor type: " + s);
    }
} // namespace RDE
