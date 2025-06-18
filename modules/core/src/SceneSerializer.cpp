#include "SceneSerializer.h"
#include "Entity.h"
#include "Components.h"
#include "Serialization.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

// Helper to write a component to the YAML emitter
template<typename T>
static void SerializeComponent(YAML::Emitter &out, RDE::Entity entity, const std::string &component_name) {
    if (entity.has_component<T>()) {
        out << YAML::Key << component_name;
        out << YAML::Value << entity.get_component<T>(); // This will use the YAML::convert specializations
    }
}

namespace RDE {
    // Overload operator<< for our components
    YAML::Emitter &operator<<(YAML::Emitter &out, const TagComponent &tc) {
        out << YAML::BeginMap;
        out << YAML::Key << "Tag" << YAML::Value << tc.tag;
        out << YAML::EndMap;
        return out;
    }

    // ... similar for TransformComponent, SpriteRendererComponent
    // TransformComponent
    YAML::Emitter &operator<<(YAML::Emitter &out, const TransformComponent &tc) {
        out << YAML::BeginMap;
        out << YAML::Key << "Translation" << YAML::Value << tc.translation;
        out << YAML::Key << "Rotation" << YAML::Value << tc.rotation;
        out << YAML::Key << "Scale" << YAML::Value << tc.scale;
        out << YAML::EndMap;
        return out;
    }

    SceneSerializer::SceneSerializer(const std::shared_ptr<Scene> &scene)
        : m_scene(scene) {
    }

    void SceneSerializer::serialize(const std::string &filepath, SerializeEntityFn serialize_callback) {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled Scene"; // TODO: Scene name
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        m_scene->get_registry().view<entt::entity>().each([&](auto entity_id) {
            Entity entity = {entity_id, m_scene.get()};
            if (!entity) return;

            out << YAML::BeginMap; // Entity map
            out << YAML::Key << "Entity" << YAML::Value << (uint64_t) entity_id; // Use ID as unique key

            SerializeComponent<TagComponent>(out, entity, "TagComponent");
            SerializeComponent<TransformComponent>(out, entity, "TransformComponent");
            // Add other components here...

            // --- HOOK FOR EXTERNAL SERIALIZATION ---
            if (serialize_callback) {
                serialize_callback(out, entity);
            }

            out << YAML::EndMap; // End Entity map
        });

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();
    }


    bool SceneSerializer::deserialize(const std::string &filepath, DeserializeEntityFn deserialize_callback) {
        std::ifstream stream(filepath);
        std::stringstream str_stream;
        str_stream << stream.rdbuf();

        YAML::Node data = YAML::Load(str_stream.str());
        if (!data["Scene"]) return false; // File is not a scene file

        m_scene->clear(); // Clear existing scene

        auto entities = data["Entities"];
        if (entities) {
            for (auto entity_node: entities) {
                uint64_t uuid = entity_node["Entity"].as<uint64_t>(); // TODO: Use a real UUID system later

                std::string name;
                auto tag_component = entity_node["TagComponent"];
                if (tag_component)
                    name = tag_component["Tag"].as<std::string>();

                Entity deserialized_entity = m_scene->create_entity(name);

                auto transform_component = entity_node["TransformComponent"];
                if (transform_component) {
                    auto &tc = deserialized_entity.get_component<TransformComponent>();
                    tc.translation = transform_component["Translation"].as<glm::vec3>();
                    tc.rotation = transform_component["Rotation"].as<glm::vec3>();
                    tc.scale = transform_component["Scale"].as<glm::vec3>();
                }

                // --- HOOK FOR EXTERNAL DESERIALIZATION ---
                if (deserialize_callback) {
                    deserialize_callback(entity_node, deserialized_entity);
                }
            }
        }
        return true;
    }
} // namespace RDE
