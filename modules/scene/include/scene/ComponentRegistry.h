#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <entt/entity/registry.hpp>
#include <imgui.h>

namespace RDE {
    struct ComponentDescriptor {
        std::string name;
        size_t type_hash = 0;
        std::function<bool(entt::registry&, entt::entity)> has;
        std::function<void(entt::registry&, entt::entity)> add_default;
        std::function<void(entt::registry&, entt::entity)> remove;
        std::function<void(entt::registry&, entt::entity)> draw;
    };

    class ComponentRegistryGui {
    public:
        static ComponentRegistryGui& instance() {
            static ComponentRegistryGui inst;
            return inst;
        }

        template<typename T>
        void register_component(const std::string &display_name,
                                std::function<void(entt::registry&, entt::entity)> draw,
                                std::function<void(entt::registry&, entt::entity)> add_default,
                                std::function<void(entt::registry&, entt::entity)> remove = {}) {
            ComponentDescriptor d{};
            d.name = display_name;
            d.type_hash = typeid(T).hash_code();
            d.has = [](entt::registry &reg, entt::entity e){ return reg.all_of<T>(e); };
            d.add_default = std::move(add_default);
            d.remove = remove ? std::move(remove) : [](entt::registry &reg, entt::entity e){ reg.remove<T>(e); };
            d.draw = std::move(draw);
            m_descriptors.emplace_back(std::move(d));
        }

        const std::vector<ComponentDescriptor>& descriptors() const { return m_descriptors; }

        // Utility UI helpers
        void draw_add_component_popup(entt::registry &reg, entt::entity e) {
            if (ImGui::BeginPopup("AddComponentPopup")) {
                for (auto &d : m_descriptors) {
                    if (!d.has(reg, e)) {
                        if (ImGui::MenuItem(d.name.c_str())) {
                            d.add_default(reg, e);
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
                ImGui::EndPopup();
            }
        }

    private:
        std::vector<ComponentDescriptor> m_descriptors;
    };
}
