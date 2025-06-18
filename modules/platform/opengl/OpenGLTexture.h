// RDE_Project/modules/platform/opengl/OpenGLTexture.h
#pragma once

#include "Renderer/Texture.h"
#include <glad/gl.h>
namespace RDE {
    class OpenGLTexture2D : public Texture2D {
    public:
        OpenGLTexture2D(const std::string &path);

        OpenGLTexture2D(uint32_t width, uint32_t height);

        virtual ~OpenGLTexture2D();

        uint32_t get_width() const override { return m_width; }

        uint32_t get_height() const override { return m_height; }

        uint32_t get_renderer_id() const override { return m_renderer_id; }

        void bind(uint32_t slot = 0) const override;

        void set_data(void* data, uint32_t size) override;

        bool operator==(const Texture &other) const override {
            return m_renderer_id == ((OpenGLTexture2D &) other).m_renderer_id;
        }

    private:
        uint32_t m_width, m_height;
        GLuint m_renderer_id;
        GLenum m_internal_format, m_data_format;
    };
}