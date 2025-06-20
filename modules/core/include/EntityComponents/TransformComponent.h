// RDE_Project/modules/core/include/Components/TransformComponent.h
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RDE {
    struct TransformComponent {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f}; // In radians
        glm::vec3 scale = {1.0f, 1.0f, 1.0f};

        TransformComponent() = default;

        TransformComponent(const TransformComponent &) = default;

        TransformComponent(const glm::vec3 &position) : position(position) {}

        glm::mat4 get_transform() const {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), rotation.x, {1, 0, 0})
                            * glm::rotate(glm::mat4(1.0f), rotation.y, {0, 1, 0})
                            * glm::rotate(glm::mat4(1.0f), rotation.z, {0, 0, 1});

            return glm::translate(glm::mat4(1.0f), position)
                   * rot
                   * glm::scale(glm::mat4(1.0f), scale);
        }
    };
}