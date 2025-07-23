#include "assets/MeshMtlLoader.h"
#include "assets/AssetComponentTypes.h"

#include <variant>

namespace RDE {
    struct MtlData {
        std::string name;

        // PBR-esque properties we will try to fill
        std::variant<glm::vec4, std::string> albedo = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
        std::variant<float, std::string> roughness = 1.0f;
        std::variant<float, std::string> metallic = 0.0f;
        std::variant<glm::vec3, std::string> emissive = glm::vec3(0.0f);
        std::variant<float, std::string> opacity = 1.0f;
        std::string normalMapPath;

        // Legacy properties we parse for potential conversion
        glm::vec3 legacy_ambient_color = glm::vec3(0.2f);   // Ka
        glm::vec3 legacy_specular_color = glm::vec3(1.0f);  // Ks
        float legacy_index_of_refraction = 1.0f;           // Ni
    };

    static std::vector<MtlData> parse_mtl_file(const std::string &uri) {
        std::vector<MtlData> materials;
        std::ifstream file(uri);

        if (!file.is_open()) {
            RDE_CORE_ERROR("Failed to open MTL file: {}", uri);
            return materials;
        }

        std::string line;
        MtlData* currentMaterial = nullptr;
        std::filesystem::path base_path = std::filesystem::path(uri).parent_path();

        while (std::getline(file, line)) {
            // Trim leading whitespace for robustness
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            // Trim trailing whitespace
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty() || line[0] == '#') {
                continue; // Skip comments and empty lines
            }

            std::stringstream ss(line);
            std::string keyword;
            ss >> keyword;

            if (keyword == "newmtl") {
                materials.emplace_back();
                currentMaterial = &materials.back();
                ss >> std::ws; // Consume whitespace
                std::getline(ss, currentMaterial->name);
            }
            else if (currentMaterial) { // Only process if we are inside a material definition
                if (keyword == "Ka") { // Ambient Color
                    ss >> currentMaterial->legacy_ambient_color.r >> currentMaterial->legacy_ambient_color.g >> currentMaterial->legacy_ambient_color.b;
                }
                else if (keyword == "Kd") { // Diffuse Color -> Albedo Color
                    glm::vec4 color;
                    ss >> color.r >> color.g >> color.b;
                    if (auto* current_opacity = std::get_if<float>(&currentMaterial->opacity)) {
                        color.a = *current_opacity;
                    }
                    currentMaterial->albedo = color;
                }
                else if (keyword == "Ks") { // Specular Color
                    ss >> currentMaterial->legacy_specular_color.r >> currentMaterial->legacy_specular_color.g >> currentMaterial->legacy_specular_color.b;
                }
                else if (keyword == "Ke") { // Emissive Color
                    glm::vec3 emissiveColor;
                    ss >> emissiveColor.r >> emissiveColor.g >> emissiveColor.b;
                    currentMaterial->emissive = emissiveColor;
                }
                else if (keyword == "Ns") { // Shininess -> Roughness
                    float shininess;
                    ss >> shininess;
                    // A common, physically-based approximation. Clamp to prevent division by zero.
                    currentMaterial->roughness = std::sqrt(2.0f / (shininess + 2.0f));
                }
                else if (keyword == "Ni") { // Index of Refraction
                    ss >> currentMaterial->legacy_index_of_refraction;
                }
                else if (keyword == "d") { // Dissolve -> Opacity
                    float opacity;
                    ss >> opacity;
                    currentMaterial->opacity = opacity;
                    if(auto* color = std::get_if<glm::vec4>(&currentMaterial->albedo)) {
                        color->a = opacity;
                    }
                }
                else if (keyword == "Tr") { // Transparency (inverse of d)
                    float transparency;
                    ss >> transparency;
                    float opacity = 1.0f - transparency;
                    currentMaterial->opacity = opacity;
                    if(auto* color = std::get_if<glm::vec4>(&currentMaterial->albedo)) {
                        color->a = opacity;
                    }
                }
                else if (keyword == "map_Kd") { // Albedo Texture
                    std::string path_str;
                    ss >> std::ws; std::getline(ss, path_str);
                    currentMaterial->albedo = (base_path / path_str).lexically_normal().string();
                }
                else if (keyword == "map_Ns") { // Roughness Texture (from specular exponent map)
                    std::string path_str;
                    ss >> std::ws; std::getline(ss, path_str);
                    currentMaterial->roughness = (base_path / path_str).lexically_normal().string();
                }
                else if (keyword == "map_Pm") { // PBR Metallic Map (often from custom exporters)
                    std::string path_str;
                    ss >> std::ws; std::getline(ss, path_str);
                    currentMaterial->metallic = (base_path / path_str).lexically_normal().string();
                }
                else if (keyword == "map_d") { // Opacity/Alpha Texture
                    std::string path_str;
                    ss >> std::ws; std::getline(ss, path_str);
                    currentMaterial->opacity = (base_path / path_str).lexically_normal().string();
                }
                else if (keyword == "map_Ke") { // Emissive Texture
                    std::string path_str;
                    ss >> std::ws; std::getline(ss, path_str);
                    currentMaterial->emissive = (base_path / path_str).lexically_normal().string();
                }
                else if (keyword == "map_Bump" || keyword == "bump" || keyword == "norm") {
                    std::string path_str;
                    // Keywords can have options like -bm. We just want the path.
                    // A simple approach is to find the first argument that doesn't start with '-'.
                    std::string temp_arg;
                    while(ss >> temp_arg) {
                        if(temp_arg[0] != '-') {
                            path_str = temp_arg;
                            break;
                        }
                    }
                    if (!path_str.empty()) {
                        currentMaterial->normalMapPath = (base_path / path_str).lexically_normal().string();
                    }
                }
            }
        }
        return materials;
    }

    std::vector<std::string> MeshMtlLoader::get_dependencies(const std::string &uri) const {
        std::vector<std::string> deps;
        auto parsed = parse_mtl_file(uri);
        std::filesystem::path base_path = std::filesystem::path(uri).parent_path();

        for(const auto& mat : parsed) {
            if(const auto* path = std::get_if<std::string>(&mat.albedo)) {
                deps.push_back((base_path / *path).lexically_normal().string());
            }
            if(const auto* path = std::get_if<std::string>(&mat.roughness)) {
                deps.push_back((base_path / *path).lexically_normal().string());
            }
            if(const auto* path = std::get_if<std::string>(&mat.metallic)) {
                deps.push_back((base_path / *path).lexically_normal().string());
            }
            if(const auto* path = std::get_if<std::string>(&mat.emissive)) {
                deps.push_back((base_path / *path).lexically_normal().string());
            }
            if(const auto* path = std::get_if<std::string>(&mat.opacity)) {
                deps.push_back((base_path / *path).lexically_normal().string());
            }
            if(!mat.normalMapPath.empty()) {
                deps.push_back((base_path / mat.normalMapPath).lexically_normal().string());
            }
        }
        return deps;
    }

    AssetID MeshMtlLoader::load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const {
        std::vector<MtlData> parsedMaterials = parse_mtl_file(uri);

        if (parsedMaterials.empty()) {
            RDE_CORE_WARN("MTL file '{}' was empty or failed to parse.", uri);
            return nullptr;
        }

        for (const auto& mtlData : parsedMaterials) {
            // Create a unique URI for this specific material for caching
            std::string material_uri = uri + "#" + mtlData.name;

            // Don't re-create if it's already in the database from another load
            if (manager.get_loaded_asset(material_uri)) continue;

            auto& registry = db.get_registry();
            entt::entity entity_id = registry.create();

            AssetMaterial materialComponent;

            // Handle Albedo (either a color or a texture)
            if (const auto* color = std::get_if<glm::vec4>(&mtlData.albedo)) {
                materialComponent.parameters.add<glm::vec4>("p:albedo_color", *color);
            } else if (const auto* path = std::get_if<std::string>(&mtlData.albedo)) {
                // Here we state a dependency. The AssetManager MUST have already loaded this texture.
                materialComponent.texture_bindings["t_albedo"] = manager.get_loaded_asset(*path);
            }

            // Handle Roughness
            if (const auto* value = std::get_if<float>(&mtlData.roughness)) {
                materialComponent.parameters.add<float>("p:roughness", *value);
            } else if (const auto* path = std::get_if<std::string>(&mtlData.roughness)) {
                // Here we state a dependency. The AssetManager MUST have already loaded this texture.
                materialComponent.texture_bindings["t_roughness"] = manager.get_loaded_asset(*path);
            }

            // Handle Metallic
            if (const auto* value = std::get_if<float>(&mtlData.metallic)) {
                materialComponent.parameters.add<float>("p:metallic", *value);
            } else if (const auto* path = std::get_if<std::string>(&mtlData.metallic)) {
                // Here we state a dependency. The AssetManager MUST have already loaded this texture.
                materialComponent.texture_bindings["t_metallic"] = manager.get_loaded_asset(*path);
            }

            // Handle Emissive
            if (const auto* color = std::get_if<glm::vec3>(&mtlData.emissive)) {
                materialComponent.parameters.add<glm::vec3>("p:emissive_color", *color);
            } else if (const auto* path = std::get_if<std::string>(&mtlData.emissive)) {
                // Here we state a dependency. The AssetManager MUST have already loaded this texture.
                materialComponent.texture_bindings["t_emissive"] = manager.get_loaded_asset(*path);
            }

            // Handle Opacity
            if (const auto* value = std::get_if<float>(&mtlData.opacity)) {
                materialComponent.parameters.add<float>("p:opacity", *value);
            } else if (const auto* path = std::get_if<std::string>(&mtlData.opacity)) {
                // Here we state a dependency. The AssetManager MUST have already loaded this texture.
                materialComponent.texture_bindings["t_opacity"] = manager.get_loaded_asset(*path);
            }

            // Handle Normal Map
            if (!mtlData.normalMapPath.empty()) {
                materialComponent.texture_bindings["t_normal"] = manager.get_loaded_asset(mtlData.normalMapPath);
            }

            // Legacy properties (if they were used, we can convert them)
            if (mtlData.legacy_ambient_color != glm::vec3(0.2f)) {
                materialComponent.parameters.add<glm::vec3>("p:legacy_ambient_color", mtlData.legacy_ambient_color);
            }
            if (mtlData.legacy_specular_color != glm::vec3(1.0f)) {
                materialComponent.parameters.add<glm::vec3>("p:legacy_specular_color", mtlData.legacy_specular_color);
            }
            if (mtlData.legacy_index_of_refraction != 1.0f) {
                materialComponent.parameters.add<float>("p:legacy_index_of_refraction", mtlData.legacy_index_of_refraction);
            }

            registry.emplace<AssetMaterial>(entity_id, std::move(materialComponent));
            registry.emplace<AssetFilepath>(entity_id, material_uri);
            registry.emplace<AssetName>(entity_id, mtlData.name);

            manager.add_to_cache(material_uri, std::make_shared<AssetID_Data>(entity_id, material_uri));
        }
        std::string first_material_uri = uri + "#" + parsedMaterials.front().name;
        return manager.get_loaded_asset(first_material_uri);
    }

    std::vector<std::string> MeshMtlLoader::get_supported_extensions() const {
        return {".mtl"};
    }
}
