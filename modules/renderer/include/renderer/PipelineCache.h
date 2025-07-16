// renderer/PipelineCache.h
#pragma once

#include "ral/Device.h" // Your Renderer Abstraction
#include "ral/Common.h"
#include "assets/AssetManager.h"
#include "assets/AssetComponentTypes.h"

#include <unordered_map>


namespace RDE {
    using ShaderFeatureMask = uint64_t;

    class PipelineCache {
    public:
        PipelineCache(AssetManager &asset_manager, RAL::Device &device)
            : m_asset_manager(asset_manager), m_device(device), m_cache() {
        }

        // The main interface for the renderer.
        RAL::PipelineHandle getPipeline(const AssetID &shader_def_id, ShaderFeatureMask feature_mask) {
            // 1. Create a unique key for this specific pipeline variant.
            PipelineVariantKey key = {shader_def_id->entity_id, feature_mask};
            if (auto it = m_cache.find(key); it != m_cache.end()) {
                return it->second;
            }

            // 2. Cache miss: We need to build it.
            return buildAndCachePipeline(shader_def_id, feature_mask, key);
        }

        struct PipelineVariantKey {
            entt::entity shader_def_entity;
            ShaderFeatureMask mask;

            bool operator==(const PipelineVariantKey &other) const {
                return shader_def_entity == other.shader_def_entity && mask == other.mask;
            }
        };

    private:
        // Hash struct for PipelineVariantKey...

        RAL::PipelineHandle buildAndCachePipeline(AssetID shader_def_id, ShaderFeatureMask mask,
                                                  const PipelineVariantKey &key);

        AssetManager &m_asset_manager;
        RAL::Device &m_device;
        std::unordered_map<PipelineVariantKey, RAL::PipelineHandle> m_cache;
    };
}
