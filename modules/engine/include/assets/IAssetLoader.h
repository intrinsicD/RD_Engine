#pragma once

#include <typeindex>
#include <entt/entity/registry.hpp>

namespace RDE{
    class IAssetLoader {
    public:
        virtual ~IAssetLoader() = default;

        // The main loading function. It's the loader's job to create the entity
        // and populate it with the correct primary component.
        virtual entt::entity load(const std::string& uri, entt::registry& registry) = 0;

        // It checks if a given asset is a valid, complete asset of the type this loader manages.
        [[nodiscard]] virtual bool verify(entt::registry& registry, entt::entity entity) const = 0;

        // Gets the primary asset type this loader produces (e.g., AssetImageCpu)
        [[nodiscard]] virtual std::type_index get_asset_type() const = 0;

        [[nodiscard]] virtual const std::vector<std::string>& get_supported_extensions() const = 0;
    };

    // --- The Typed Template Bridge ---
    // Concrete loaders will inherit from this.
    template <typename AssetType>
    class TypedAssetLoader : public IAssetLoader {
    public:
        // Constructor that takes the extensions the concrete loader supports.
        explicit TypedAssetLoader(std::vector<std::string> extensions)
            : m_supported_extensions(std::move(extensions)) {}

        // Implement the untyped base 'load' by calling a new typed method.
        // This is the "Template Method" pattern. This method is final to ensure the pattern is followed.
        entt::entity load(const std::string& uri, entt::registry& registry) final {
            entt::entity new_entity = registry.create();
            load_typed(uri, registry, new_entity);
            return new_entity;
        }

        [[nodiscard]] bool verify(entt::registry& registry, entt::entity entity) const override {
            return registry.all_of<AssetType>(entity);
        }

        // Return the specific type this loader works with.
        [[nodiscard]] std::type_index get_asset_type() const final {
            return std::type_index(typeid(AssetType));
        }

        [[nodiscard]] const std::vector<std::string>& get_supported_extensions() const final {
            return m_supported_extensions;
        }

    protected:
        // Concrete loaders MUST implement this strongly-typed function.
        virtual void load_typed(const std::string& uri, entt::registry& registry, entt::entity asset_id) = 0;

    private:
        std::vector<std::string> m_supported_extensions;
    };
}