#include "renderer/ShaderReflector.h"
#include "core/Log.h"

#include <spirv_cross/spirv_cross.hpp>

namespace RDE {
    namespace ShaderReflector {
        // Helper to convert from SPIRV-Cross type to our RAL enum
        RAL::DescriptorType spirv_to_ral_type(spirv_cross::SPIRType::BaseType type) {
            switch (type) {
                case spirv_cross::SPIRType::SampledImage:
                    return RAL::DescriptorType::CombinedImageSampler;
                case spirv_cross::SPIRType::Struct:
                    // Assuming UBOs are structs. Can be refined for Storage Buffers.
                    return RAL::DescriptorType::UniformBuffer;
                default:
                    // Add more types as you support them (StorageImage, etc.)
                    return RAL::DescriptorType::UniformBuffer;
            }
        }

        // The implementation of the reflection logic
        ReflectedLayout reflect(const std::map<RAL::ShaderStage, const std::vector<char> *> &shader_stages) {
            ReflectedLayout layout;
            RAL::ShaderStage accumulated_push_constant_stages = RAL::ShaderStage::None;
            uint32_t max_push_constant_size = 0;

            // --- Define the lambda ONCE here ---
            // It now takes `current_stage` as a parameter.
            auto process_resources = [&](const spirv_cross::Compiler &compiler,
                                         const spirv_cross::SmallVector<spirv_cross::Resource> &resource_list,
                                         RAL::ShaderStage current_stage) {
                // <-- EXPLICIT PARAMETER
                for (const auto &res: resource_list) {
                    uint32_t set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
                    uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
                    auto &layout_desc = layout.setLayouts[set];
                    auto it = std::find_if(layout_desc.bindings.begin(), layout_desc.bindings.end(),
                                           [binding](const auto &b) { return b.binding == binding; });
                    if (it != layout_desc.bindings.end()) {
                        it->stages |= current_stage; // Use the parameter
                    } else {
                        RAL::DescriptorSetLayoutBinding new_binding;
                        new_binding.binding = binding;
                        new_binding.stages = current_stage; // Use the parameter
                        new_binding.type = spirv_to_ral_type(compiler.get_type(res.type_id).basetype);
                        layout_desc.bindings.push_back(new_binding);
                    }
                }
            };

            for (const auto &[stage, bytecode]: shader_stages) {
                spirv_cross::Compiler compiler(
                    reinterpret_cast<const uint32_t *>(bytecode->data()),
                    bytecode->size() / sizeof(uint32_t)
                );
                spirv_cross::ShaderResources resources = compiler.get_shader_resources();

                // --- Call the lambda with the current stage ---
                process_resources(compiler, resources.uniform_buffers, stage);
                process_resources(compiler, resources.sampled_images, stage);
                //TODO ... process other resource types like storage_buffers here if needed...

                if (!resources.push_constant_buffers.empty()) {
                    accumulated_push_constant_stages |= stage;
                    const auto &push_resource = resources.push_constant_buffers[0];
                    auto ranges = compiler.get_active_buffer_ranges(push_resource.id);
                    for (const auto &range: ranges) {
                        // Find the largest offset + size to determine total size
                        if (range.offset + range.range > max_push_constant_size) {
                            max_push_constant_size = range.offset + range.range;
                        }
                    }
                }
            }

            // Finalize the push constant range
            if (max_push_constant_size <= 0) {
                RDE_CORE_ERROR("No valid push constant ranges found in the provided shader stages.");
                return {};
            }

            RAL::PushConstantRange range;
            range.stages = accumulated_push_constant_stages;
            range.offset = 0; // Offset is almost always 0
            range.size = max_push_constant_size;
            layout.pushConstantRanges.emplace_back(range);

            return layout;
        }
    } // namespace ShaderReflector
}
