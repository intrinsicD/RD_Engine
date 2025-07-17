// renderer/PipelineCache.h
#pragma once

#include "ral/Device.h" // Your Renderer Abstraction
#include "ral/Common.h"
#include "assets/AssetManager.h"
#include "assets/AssetComponentTypes.h"

#include <unordered_map>
#include <functional>

namespace RDE {
    using ShaderFeatureMask = uint64_t;

    struct PipelineVariantKey {
        entt::entity shader_def_entity;
        ShaderFeatureMask mask;

        PipelineVariantKey() = default;

        PipelineVariantKey(entt::entity entity, ShaderFeatureMask feature_mask)
                : shader_def_entity(entity), mask(feature_mask) {}

        bool operator==(const PipelineVariantKey &other) const {
            return shader_def_entity == other.shader_def_entity && mask == other.mask;
        }

        bool operator!=(const PipelineVariantKey &other) const {
            return !(*this == other);
        }
    };
}

// And the hash specialization
namespace std {
    template<>
    struct hash<RDE::PipelineVariantKey> {
        size_t operator()(const RDE::PipelineVariantKey &key) const {
            size_t h1 = std::hash<entt::entity>()(key.shader_def_entity);
            size_t h2 = std::hash<RDE::ShaderFeatureMask>()(key.mask);
            return h1 ^ (h2 << 1);
        }
    };
}

namespace RDE {
    class PipelineCache {
    public:
        PipelineCache(AssetManager &asset_manager, RAL::Device &device);
        ~PipelineCache();

        // The main interface for the renderer.
        RAL::PipelineHandle getPipeline(const AssetID &shader_def_id, ShaderFeatureMask feature_mask);
    private:
        struct CachedPipeline {
            RAL::PipelineHandle pipeline;
            std::vector<RAL::DescriptorSetLayoutHandle> setLayouts;
            std::vector<RAL::ShaderHandle> shaderModules;
        };

        RAL::PipelineHandle buildAndCachePipeline(AssetID shader_def_id, ShaderFeatureMask mask,
                                                  const PipelineVariantKey &key);

        RAL::ShaderHandle find_shader_handle(const std::vector<RAL::ShaderHandle> &handles, RAL::ShaderStage stage) const;

        AssetManager &m_asset_manager;
        RAL::Device &m_device;
        std::unordered_map<PipelineVariantKey, CachedPipeline> m_cache;
    };
}
