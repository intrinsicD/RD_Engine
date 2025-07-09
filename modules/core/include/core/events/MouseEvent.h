// RDE_Project/modules/core/include/Events/MouseEvent.h

#pragma once

#include "Event.h"
#include "core/MouseCodes.h"

namespace RDE {

    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(const float x, const float y)
                : m_mouse_x(x), m_mouse_y(y) {
        }

        float get_x() const { return m_mouse_x; }

        float get_y() const { return m_mouse_y; }

        std::string to_string() const override {
            return "MouseMovedEvent: " + std::to_string(m_mouse_x) + ", " + std::to_string(m_mouse_y);
        }

        int get_category_flags() const override {
            return EventCategory::EventCategoryInput | EventCategory::EventCategoryMouse;
        }

        EVENT_CLASS_TYPE(MouseMoved)

    private:
        float m_mouse_x, m_mouse_y;
    };

    class MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(const float x_offset, const float y_offset)
                : m_x_offset(x_offset), m_y_offset(y_offset) {
        }

        float get_x_offset() const { return m_x_offset; }

        float get_y_offset() const { return m_y_offset; }

        std::string to_string() const override {
            return "MouseScrolledEvent: " + std::to_string(get_x_offset()) + ", " + std::to_string(get_y_offset());
        }

        int get_category_flags() const override {
            return EventCategory::EventCategoryInput | EventCategory::EventCategoryKeyboard;
        }

        EVENT_CLASS_TYPE(MouseScrolled)

    private:
        float m_x_offset, m_y_offset;
    };

    class MouseButtonEvent : public Event {
    public:
        int get_mouse_button() const { return m_button; }

        int get_category_flags() const override {
            return EventCategory::EventCategoryInput | EventCategory::EventCategoryMouse;
        }

        bool is_left_button() const {
            return m_button == 0; // Assuming 0 is the left mouse button
        }

        bool is_right_button() const {
            return m_button == 1; // Assuming 1 is the right mouse button
        }

        bool is_middle_button() const {
            return m_button == 2; // Assuming 2 is the middle mouse button
        }

    protected:
        MouseButtonEvent(const MouseButton button)
                : m_button(button) {
        }

        MouseButton m_button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent {
    public:
        MouseButtonPressedEvent(const MouseButton button)
                : MouseButtonEvent(button) {
        }

        std::string to_string() const override {
            return "MouseButtonPressedEvent: " + std::to_string(m_button);
        }

        int get_category_flags() const override {
            return EventCategory::EventCategoryInput | EventCategory::EventCategoryMouseButton;
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent {
    public:
        MouseButtonReleasedEvent(const MouseButton button)
                : MouseButtonEvent(button) {
        }

        std::string to_string() const override {
            return "MouseButtonReleasedEvent: " + std::to_string(m_button);
        }

        int get_category_flags() const override {
            return EventCategory::EventCategoryInput | EventCategory::EventCategoryMouseButton;
        }

        EVENT_CLASS_TYPE(MouseButtonReleased)
    };

}