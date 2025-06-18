#include "OpenGLTexture.h"
#include "OpenGLDebug.h"
#include "Core/Log.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
namespace RDE {
    std::shared_ptr<Texture2D> Texture2D::Create(const std::string &path) {
        return std::make_shared<OpenGLTexture2D>(path);
    }

    std::shared_ptr<Texture2D> Texture2D::Create(uint32_t width, uint32_t height) {
        return std::make_shared<OpenGLTexture2D>(width, height);
    }

    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
            : m_width(width), m_height(height), m_internal_format(GL_RGBA8), m_data_format(GL_RGBA) {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_renderer_id);
        GL_CHECK_ERROR();
        glTextureStorage2D(m_renderer_id, 1, m_internal_format, m_width, m_height);
        GL_CHECK_ERROR();
        glTextureParameteri(m_renderer_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        GL_CHECK_ERROR();
        glTextureParameteri(m_renderer_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        GL_CHECK_ERROR();
        glTextureParameteri(m_renderer_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        GL_CHECK_ERROR();
        glTextureParameteri(m_renderer_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
        GL_CHECK_ERROR();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GL_CHECK_ERROR();
    }


    OpenGLTexture2D::OpenGLTexture2D(const std::string &path)
            : m_width(0), m_height(0), m_renderer_id(0) {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        RDE_CORE_ASSERT(data, "Failed to load image!");
        m_width = width;
        m_height = height;

        if (channels == 4) {
            m_internal_format = GL_RGBA8;
            m_data_format = GL_RGBA;
        } else if (channels == 3) {
            m_internal_format = GL_RGB8;
            m_data_format = GL_RGB;
        } else {
            RDE_CORE_ASSERT(false, "Unsupported number of channels!");
        }

        RDE_CORE_ASSERT(m_internal_format & m_data_format, "Image format not supported!");

        glCreateTextures(GL_TEXTURE_2D, 1, &m_renderer_id);
        GL_CHECK_ERROR();
        glTextureStorage2D(m_renderer_id, 1, m_internal_format, m_width, m_height);
        GL_CHECK_ERROR();
        glTextureParameteri(m_renderer_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        GL_CHECK_ERROR();
        glTextureParameteri(m_renderer_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        GL_CHECK_ERROR();
        glTextureParameteri(m_renderer_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        GL_CHECK_ERROR();
        glTextureParameteri(m_renderer_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
        GL_CHECK_ERROR();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GL_CHECK_ERROR();
        glTextureSubImage2D(m_renderer_id, 0, 0, 0, m_width, m_height, m_data_format, GL_UNSIGNED_BYTE, data);
        GL_CHECK_ERROR();

        glGenerateTextureMipmap(m_renderer_id);
        GL_CHECK_ERROR();

        stbi_image_free(data);
    }

    OpenGLTexture2D::~OpenGLTexture2D() {
        glDeleteTextures(1, &m_renderer_id);
        GL_CHECK_ERROR();
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const {
        glBindTextureUnit(slot, m_renderer_id);
        GL_CHECK_ERROR();
    }
}