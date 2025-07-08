#pragma once

namespace RDE{
    class SystemDependencyBuilder;

    class ISystem {
    public:
        virtual ~ISystem() = default;

        // Called once at the start of the application
        virtual void init() = 0;

        // Called every frame before rendering
        virtual void update(float delta_time) = 0;

        // Called once at the end of the application
        virtual void shutdown() = 0;

    private:
        friend class SystemGraph;

        virtual void declare_dependencies(SystemDependencyBuilder &builder) = 0;
    };
}