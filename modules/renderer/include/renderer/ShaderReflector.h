#pragma once

#include "ral/Resources.h"
#include <vector>
#include <map>

namespace RDE {

    // A struct to hold the results of reflection.
    struct ReflectedLayout {
        std::map<uint32_t, RAL::DescriptorSetLayoutDescription> setLayouts; // Keyed by set number
        std::vector<RAL::PushConstantRange> pushConstantRanges;
    };

    namespace ShaderReflector {
        // The main function. Takes a map of shader stages to their bytecode
        // and returns the complete layout description.
        ReflectedLayout reflect(const std::map<RAL::ShaderStage, const std::vector<char>*>& shader_stages);
    }

} // namespace RDE