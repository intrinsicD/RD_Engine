#pragma once

#include "Entity.h"
#include "Scene.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <typeindex>

namespace RDE {
    // A function that draws the UI for a component on a given entity.
    using ComponentDrawFn = std::function<void(Entity)>;

    // A function that adds a component to an entity.
    using ComponentAddFn = std::function<void(Entity)>;

    struct ComponentUIFunctions {
        std::string name;
        ComponentDrawFn draw_function;
        ComponentAddFn add_function;
    };

    class ComponentUIRegistry {
    public:
        // Registers a component type with the UI system.
        template<typename T>
        static void RegisterComponent(const std::string &name, ComponentDrawFn draw_fn) {
            std::type_index type_index(typeid(T));
            s_component_ui_map[type_index] = {
                name,
                draw_fn,
                [](Entity entity) { entity.add_component<T>(); }
            };
        }

        static const ComponentUIFunctions *GetUIFunctions(const std::type_index &type_index) {
            if (s_component_ui_map.count(type_index)) {
                return &s_component_ui_map.at(type_index);
            }
            return nullptr;
        }

        template<typename T>
        static void Draw(Scene *scene, Entity entity) {
            if (scene->get_registry().all_of<T>(entity)) {
                const std::type_index type_index = typeid(scene->get_registry().get<T>(entity));
                if (auto ui_funcs = ComponentUIRegistry::GetUIFunctions(type_index)) {
                    // If found, call the registered draw function
                    ui_funcs->draw_function(entity);
                }
            }
        }

        static const auto &GetRegistry() { return s_component_ui_map; }

    private:
        // A map from a component's type_index to its UI functions.
        static std::unordered_map<std::type_index, ComponentUIFunctions> s_component_ui_map;
    };
}
