// RDE_Project/modules/core/include/Scene.h
#pragma once

#include "ISystem.h"

#include <entt/entt.hpp>

namespace RDE {
    class Entity; // Forward declaration
    class JobSystem;
    class IRenderer;
    class AssetManager;

    class Scene {
    public:
        Scene(JobSystem &job_system, IRenderer &renderer, AssetManager &asset_manager);

        ~Scene();

        Entity create_entity(const std::string &name = std::string());

        void destroy_entity(Entity entity);

        void on_update_simulation(float fixed_time_step);

        void on_update_presentation(float delta_time);

        void on_submit_render_data();

        void clear();

        entt::registry &get_registry() { return m_registry; }

        auto &get_context() { return m_registry.ctx(); }

        template<class T>
        T *attach_system(std::unique_ptr<T> system) {
            T *ptr = system.get();
            m_systems.emplace_back(std::move(system));
            ptr->on_attach(this);
            return ptr;
        }

        bool detach_system(ISystem *system);

    private:
        entt::registry m_registry;
        std::vector<std::unique_ptr<ISystem> > m_systems;

        JobSystem &m_job_system;
        IRenderer &m_renderer;
        AssetManager &m_asset_manager;

        friend class Entity;
    };
}