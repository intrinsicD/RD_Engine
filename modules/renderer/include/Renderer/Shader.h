// RDE_Project/modules/renderer/include/Renderer/Shader.h
#pragma once

#include <string>
#include <memory>
#include <glm/glm.hpp> // Include glm

class Shader {
public:
    virtual ~Shader() = default;

    virtual void Bind() const = 0;

    virtual void Unbind() const = 0;

    virtual void SetMat4(const std::string& name, const glm::mat4& matrix) = 0;

    virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;

    // Factory method for creating shaders
    static std::shared_ptr<Shader> Create(const std::string &vertexSrc, const std::string &fragmentSrc);

    static std::shared_ptr<Shader>
    CreateFromFile(const std::string &vertexFilepath, const std::string &fragmentFilepath);
};
