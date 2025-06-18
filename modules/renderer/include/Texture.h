// RDE_Project/modules/renderer/include/Renderer/Texture.h
#pragma once

#include <string>
#include <memory>

namespace RDE {
    class Texture {
    public:
        virtual ~Texture() = default;

        virtual const std::string &get_path() const = 0;

        virtual uint32_t get_width() const = 0;

        virtual uint32_t get_height() const = 0;

        virtual uint32_t get_renderer_id() const = 0;

        virtual void bind(uint32_t slot = 0) const = 0;

        virtual void set_data(void *data, uint32_t size) = 0;

        virtual bool operator==(const Texture &other) const = 0;
    };

    class Texture2D : public Texture {
    public:
        static std::shared_ptr<Texture2D> Create(const std::string &path);

        static std::shared_ptr<Texture2D> Create(uint32_t width, uint32_t height);
    };
}
