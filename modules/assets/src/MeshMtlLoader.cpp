#include "assets/MeshMtlLoader.h"
#include "assets/AssetComponentTypes.h"

namespace RDE {
    struct MtlData {
        std::variant<glm::vec4, std::string> diffuse_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Ka
        std::variant<glm::vec4, std::string> ambient_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Kd
        std::variant<glm::vec4, std::string> specular_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Ks
        float shininess{0.0f}; // Ns
        float dissolve{0.0f};   // d (dissolve factor, 0.0 = fully opaque, 1.0 = fully transparent)
        int illum{0}; // Illumination model (0 = color on and ambient off, 1 = color on and ambient on, etc.)

        std::string bump_texture_path; // map_bump or bump
        std::string name; // Name of the material
    };

    struct Mtl {
        std::vector<MtlData> mtls;
    };

    static MtlData parse_mtl_file(const std::string &uri) {
        // This function should parse the MTL file and return a MtlData struct.
        // For now, let's assume it returns an empty struct.#
        //TODO continue here...
        return MtlData{};
    }

    std::vector<std::string> MeshMtlLoader::get_dependencies(const std::string &uri) const {
    }

    AssetID MeshMtlLoader::load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const {
        // 1. Parse the .mtl file (you'll need a simple parser for this)
        //    Let's assume it returns a struct like `MtlData`.
        MtlData mtl = parse_mtl_file(uri);

        // 2. Create the asset entity
        auto &registry = db.get_registry();
        entt::entity entity_id = registry.create();

        // 3. Populate the AssetMaterial component
        AssetMaterial material;
        // You would map the parsed Ka, Kd, Ks, map_Kd etc. to your PBR properties
        if (std::holds_alternative<glm::vec4>(mtl.diffuse_color)) {
            material.parameters.add<glm::vec4>("p:albedo_color", std::get<glm::vec4>(mtl.diffuse_color));
        }else {
            auto texture_path = std::get<std::string>(mtl.diffuse_color);
            if (!texture_path.empty()) {
                material.texture_bindings["t_albedo"] = manager.get_loaded_asset(std::get<std::string>(mtl.diffuse_color));
            }
        }
        if (std::holds_alternative<glm::vec4>(mtl.ambient_color)) {
            material.parameters.add<glm::vec4>("p:ambient_color", std::get<glm::vec4>(mtl.ambient_color));
        } else {
            auto texture_path = std::get<std::string>(mtl.ambient_color);
            if (!texture_path.empty()) {
                material.texture_bindings["t_ambient"] = manager.get_loaded_asset(std::get<std::string>(mtl.ambient_color));
            }
        }
        if (std::holds_alternative<glm::vec4>(mtl.specular_color)) {
            material.parameters.add<glm::vec4>("p:specular_color", std::get<glm::vec4>(mtl.specular_color));
        } else {
            auto texture_path = std::get<std::string>(mtl.specular_color);
            if (!texture_path.empty()) {
                material.texture_bindings["t_specular"] = manager.get_loaded_asset(std::get<std::string>(mtl.specular_color));
            }
        }

        material.parameters.add<float>("p:roughness_factor", 1.0f - mtl.shininess); // Example conversion
        material.parameters.add<float>("p:dissolve_factor", mtl.dissolve); // d
        material.parameters.add<int>("p:illumination_model", mtl.illum); // Illumination model

        registry.emplace<AssetMaterial>(entity_id, std::move(material));
        registry.emplace<AssetFilepath>(entity_id, uri);
        registry.emplace<AssetName>(entity_id, std::filesystem::path(uri).filename().string());

        return std::make_shared<AssetID_Data>(entity_id, uri);
    }

    std::vector<std::string> MeshMtlLoader::get_supported_extensions() const {
        return {".mtl"};
    }
}
