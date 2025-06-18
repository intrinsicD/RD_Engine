#pragma once

#include "Scene.h"
#include <string>

// Forward-declare YAML types to avoid including yaml-cpp in this public header.
namespace YAML { class Emitter; class Node; }

namespace RDE {
    class SceneSerializer {
    public:
        // Define function signatures for our serialization hooks
        using SerializeEntityFn = std::function<void(YAML::Emitter&, Entity)>;
        using DeserializeEntityFn = std::function<void(const YAML::Node&, Entity)>;

        explicit SceneSerializer(const std::shared_ptr<Scene> &scene);

        void serialize(const std::string &filepath, SerializeEntityFn serialize_callback = nullptr);

        bool deserialize(const std::string &filepath, DeserializeEntityFn deserialize_callback = nullptr);

    private:
        std::shared_ptr<Scene> m_scene;
    };
}
