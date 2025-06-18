#pragma once

#include "Scene.h"
#include <string>

namespace RDE {
    class SceneSerializer {
    public:
        SceneSerializer(const std::shared_ptr<Scene> &scene);

        void serialize(const std::string &filepath);

        bool deserialize(const std::string &filepath);

    private:
        std::shared_ptr<Scene> m_scene;
    };
}
