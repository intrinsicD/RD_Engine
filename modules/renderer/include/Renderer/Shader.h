// RDE_Project/modules/renderer/include/Renderer/Shader.h
#pragma once

#include <string>
#include <memory>
// TODO-DESIGN: Add a math library like GLM later. For now, no math dependencies.

class Shader {
public:
    virtual ~Shader() = default;

    virtual void Bind() const = 0;

    virtual void Unbind() const = 0;

    // Factory method for creating shaders
    static std::unique_ptr<Shader> Create(const std::string &vertexSrc, const std::string &fragmentSrc);
};
