#include "VulkanTextureUploader.h"
#include "AssetComponentTypes.h"
#include "VulkanImmediateSubmit.h" // We need the concrete implementation here.
#include "ral/Resources.h"          // And the concrete device.

namespace RDE {

    void VulkanTextureUploader::process_uploads(AssetDatabase& asset_db, VulkanDevice& device) {
        auto& asset_registry = asset_db.get_registry();
        auto view = asset_registry.view<const AssetCpuTexture>(entt::exclude<AssetGpuTexture>);
        if (view.size_hint() == 0) {
            return;
        }

        // This is a concrete implementation detail, so we need the concrete device.
        auto& vk_device = static_cast<VulkanDevice&>(device);
        VulkanImmediateSubmit immediate_submit(vk_device);

        for (auto entity : view) {
            const auto& cpu_texture = view.get<const AssetCpuTexture>(entity);
            const auto& filepath = asset_registry.get<AssetFilepath>(entity); // For debug names

            // 1. Create Staging Buffer
            RAL::BufferDescription staging_desc;
            staging_desc.size = cpu_texture.data.size();
            staging_desc.usage = RAL::ResourceUsage::DYNAMIC; // So we can map it
            RAL::BufferHandle staging_handle = device.create_buffer(staging_desc);

            // 2. Copy data to staging buffer
            // A real impl would get the mapped pointer from VMA
            // void* mapped_data = ...;
            // memcpy(mapped_data, cpu_texture.data.data(), cpu_texture.data.size());

            // 3. Create final destination texture on GPU
            RAL::TextureDescription final_tex_desc;
            final_tex_desc.width = cpu_texture.width;
            final_tex_desc.height = cpu_texture.height;
            final_tex_desc.format = RAL::Format::R8G8B8A8_SRGB; // Assume for now
            final_tex_desc.usage = RAL::ResourceUsage::GPU_ONLY;
            final_tex_desc.name = filepath.path;
            RAL::TextureHandle final_tex_handle = device.create_texture(final_tex_desc);

            // 4. Record and submit copy commands
            immediate_submit.submit([&](RAL::CommandBuffer& cmd) {
                // Transition final texture from UNDEFINED to TRANSFER_DST_OPTIMAL
                RAL::BarrierInfo barrier1;
                // barrier1.texture_barriers.push_back({ ... });
                cmd.pipeline_barrier(barrier1);

                // Record the copy command
                // cmd.copy_buffer_to_texture(staging_handle, final_tex_handle, ...);

                // Transition final texture from TRANSFER_DST_OPTIMAL to SHADER_READ_ONLY_OPTIMAL
                RAL::BarrierInfo barrier2;
                // barrier2.texture_barriers.push_back({ ... });
                cmd.pipeline_barrier(barrier2);
            });

            // 5. Clean up staging buffer
            device.destroy_buffer(staging_handle);

            // 6. Add the final GPU component to the asset entity
            asset_registry.emplace<AssetGpuTexture>(entity, final_tex_handle);
            // We can now remove the CPU data if we want
            // asset_registry.remove<AssetCpuTexture>(entity);
        }
    }

}