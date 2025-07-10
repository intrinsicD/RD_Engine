#pragma once

#include "Event.h"
#include "core/KeyCodes.h"

namespace RDE {
    class KeyEvent : public Event {
    public:
        KeyCode get_key_code() const { return m_key_code; }

        int get_category_flags() const override {
            return EventCategory::EventCategoryInput | EventCategory::EventCategoryKeyboard;
        }

    protected:
        explicit KeyEvent(const KeyCode keycode) : m_key_code(keycode) {
        }

        KeyCode m_key_code;
    };

    class KeyPressedEvent : public KeyEvent {
    public:
        explicit KeyPressedEvent(const KeyCode keycode, bool is_repeat = false)
            : KeyEvent(keycode), m_is_repeat(is_repeat) {
        }

        bool is_repeat() const { return m_is_repeat; }

        std::string to_string() const override {
            return "KeyPressedEvent: " + std::to_string(m_key_code) + " (repeat = " + std::to_string(m_is_repeat) + ")";
        }

        int get_category_flags() const override {
            return EventCategory::EventCategoryInput | EventCategory::EventCategoryKeyboard;
        }

        EVENT_CLASS_TYPE(KeyPressed)

    private:
        bool m_is_repeat;
    };

    class KeyReleasedEvent : public KeyEvent {
    public:
        explicit KeyReleasedEvent(const KeyCode keycode)
            : KeyEvent(keycode) {
        }

        std::string to_string() const override {
            return "KeyReleasedEvent: " + std::to_string(m_key_code);
        }

        int get_category_flags() const override {
            return EventCategory::EventCategoryInput | EventCategory::EventCategoryKeyboard;
        }

        EVENT_CLASS_TYPE(KeyReleased)
    };
}
