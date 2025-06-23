#pragma once

#include "Events/Event.h"

namespace RDE {
    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_width(width), m_height(height) {
        }

        unsigned int get_width() const { return m_width; }

        unsigned int get_height() const { return m_height; }

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

    class WindowFileDropEvent : public Event {
    public:
        explicit WindowFileDropEvent(const std::vector<std::string> &files) : files(files) {
        }

        int get_category_flags() const override {
            return EventCategory::EventCategoryApplication;
        }

        const std::vector<std::string> &get_files() const { return files; }

        EVENT_CLASS_TYPE(WindowFileDrop)

    private:
        std::vector<std::string> files;
    };
}
