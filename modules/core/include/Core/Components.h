// RDE_Project/modules/core/include/Core/Components.h
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <memory>

namespace RDE {
    struct TagComponent {
        std::string tag;

        TagComponent() = default;

        TagComponent(const TagComponent &) = default;

        TagComponent(const std::string &tag) : tag(tag) {}
    };

    struct TransformComponent {
        glm::vec3 translation = {0.0f, 0.0f, 0.0f};
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f}; // In radians
        glm::vec3 scale = {1.0f, 1.0f, 1.0f};

        TransformComponent() = default;

        TransformComponent(const TransformComponent &) = default;

        TransformComponent(const glm::vec3 &translation) : translation(translation) {}

        glm::mat4 get_transform() const {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), rotation.x, {1, 0, 0})
                            * glm::rotate(glm::mat4(1.0f), rotation.y, {0, 1, 0})
                            * glm::rotate(glm::mat4(1.0f), rotation.z, {0, 0, 1});

            return glm::translate(glm::mat4(1.0f), translation)
                   * rot
                   * glm::scale(glm::mat4(1.0f), scale);
        }
    };

    class Texture2D;

    struct SpriteRendererComponent {
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        std::shared_ptr<Texture2D> texture;
        float tiling_factor = 1.0f;

        SpriteRendererComponent() = default;

        SpriteRendererComponent(const SpriteRendererComponent &) = default;

        SpriteRendererComponent(const glm::vec4 &color) : color(color) {}
    };
}