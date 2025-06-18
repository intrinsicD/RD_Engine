// RDE_Project/modules/core/include/Core/Events/Event.h

#pragma once

#include <string>

namespace RDE {

    enum class EventType {
        None = 0,
        WindowClose, WindowResize,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    enum EventCategory {
        None = 0,
        EventCategoryApplication = 1 << 0,
        EventCategoryInput = 1 << 1,
        EventCategoryKeyboard = 1 << 2,
        EventCategoryMouse = 1 << 3,
        EventCategoryMouseButton = 1 << 4
    };

// A macro to help define the event type in each event class.
// This is a standard pattern to avoid writing the same boilerplate code repeatedly.
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
virtual EventType get_event_type() const override { return GetStaticType(); }\
virtual const char* get_name() const override { return #type; }

    class Event {
    public:
        virtual ~Event() = default;

        bool handled = false;

        virtual EventType get_event_type() const = 0;

        virtual const char *get_name() const = 0;

        virtual int get_category_flags() const = 0;

        virtual std::string to_string() const { return get_name(); }

        bool IsInCategory(EventCategory category) {
            return get_category_flags() & category;
        }
    };

    class EventDispatcher {
    public:
        EventDispatcher(Event &event)
                : m_event(event) {
        }

        // Dispatches an event to a handler function if the event type matches.
        // T is the specific Event class (e.g., KeyPressedEvent).
        // F is a function type, typically bool(T&).
        template<typename T, typename F>
        bool dispatch(const F &func) {
            if (m_event.get_event_type() == T::GetStaticType()) {
                // Call the function, casting the base Event& to the specific type T&.
                // The result of func determines if the event is "handled".
                m_event.handled |= func(static_cast<T &>(m_event));
                return true;
            }
            return false;
        }

    private:
        Event &m_event;
    };

// A simple helper for logging events.
    inline std::ostream &operator<<(std::ostream &os, const Event &e) {
        return os << e.to_string();
    }

}