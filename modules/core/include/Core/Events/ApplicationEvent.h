#pragma once

#include "Core/Events/Event.h"

namespace RDE {

    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
                : m_width(width), m_height(height) {
        }

        unsigned int GetWidth() const { return m_width; }

        unsigned int GetHeight() const { return m_height; }

        std::string to_string() const override {
            return "WindowResizeEvent: " + std::to_string(m_width) + ", " + std::to_string(m_height);
        }

        int get_category_flags() const override {
            return EventCategory::EventCategoryApplication;
        }

        EVENT_CLASS_TYPE(WindowResize)

    private:
        unsigned int m_width, m_height;
    };

    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;

        int get_category_flags() const override {
            return EventCategory::EventCategoryApplication;
        }

        EVENT_CLASS_TYPE(WindowClose)
    };

}