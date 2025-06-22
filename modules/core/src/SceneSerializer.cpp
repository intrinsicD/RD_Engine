#include "SceneSerializer.h"
#include "Entity.h"
#include "EntityComponents/TagComponent.h"
#include "EntityComponents/TransformComponent.h"
#include "YamlUtils.h"
#include "Log.h"

#include <fstream>
#include <filesystem>
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
        out << YAML::Key << "Translation" << YAML::Value << tc.position;
        out << YAML::Key << "Rotation" << YAML::Value << tc.rotation;
        out << YAML::Key << "Scale" << YAML::Value << tc.scale;
        out << YAML::EndMap;
        return out;
    }

    SceneSerializer::SceneSerializer(const std::shared_ptr<Scene> &scene)
        : m_scene(scene) {
    }

    void SceneSerializer::serialize(const std::string &filepath, SerializeEntityFn serialize_callback) {
        try {
            std::filesystem::path path = filepath;
            std::filesystem::path directory = path.parent_path();
            if (!std::filesystem::exists(directory)) {
                RDE_CORE_INFO("Creating directory: {0}", directory.string());
                if (!std::filesystem::create_directories(directory)) {
                    RDE_CORE_ERROR("Failed to create directory for serialization: {0}", directory.string());
                    return; // Abort if directory creation fails
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            RDE_CORE_ERROR("Filesystem error: {0}", e.what());
            return;
        }

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
        if (!fout.is_open()) {
            RDE_CORE_ERROR("Failed to open file for writing: {0}", filepath);
            return; // Abort if file cannot be opened
        }
        fout << out.c_str();
        fout.close();
    }


    bool SceneSerializer::deserialize(const std::string &filepath, DeserializeEntityFn deserialize_callback) {
        if (!std::filesystem::exists(filepath)) {
            RDE_CORE_ERROR("Scene file does not exist: {0}", filepath);
            return false; // File does not exist
        }

        std::ifstream stream(filepath);
        if (!stream.is_open()) {
            RDE_CORE_ERROR("Failed to open scene file: {0}", filepath);
            return false; // Abort if file cannot be opened
        }
        std::stringstream str_stream;
        str_stream << stream.rdbuf();
        stream.close();

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
                    auto &tc = deserialized_entity.add_component<TransformComponent>();
                    tc.position = transform_component["Translation"].as<glm::vec3>();
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
