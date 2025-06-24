// RDE_Project/modules/renderer/include/Renderer/Shader.h
#pragma once

#include "assets/IAsset.h"

#include <string>
#include <memory>
#include <glm/glm.hpp> // Include glm

namespace RDE {
    class Shader : public IAsset{
    public:
        virtual ~Shader() = default;

        virtual void bind() const = 0;

        virtual void unbind() const = 0;

        virtual void set_mat4(const std::string &name, const glm::mat4 &matrix) = 0;

        virtual void set_int(const std::string &name, int value) = 0;

        virtual void set_int_array(const std::string &name, int *values, uint32_t count) = 0;

        virtual void set_float(const std::string &name, const glm::vec3 &data) = 0;

        virtual void set_float(const std::string &name, const glm::vec4 &data) = 0;

        // Factory method for creating shaders
        static std::shared_ptr<Shader> Create(const std::string &vertexSrc, const std::string &fragmentSrc);

        static std::shared_ptr<Shader>
        CreateFromFile(const std::string &vertexFilepath, const std::string &fragmentFilepath);
    };
}