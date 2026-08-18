//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include "Event.hpp"

namespace engine {
    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(const float x, const float y) : x_(x), y_(y) {}

        [[nodiscard]] float x() const { return x_; }
        [[nodiscard]] float y() const { return y_; }

        static EventType get_static_type() { return EventType::MouseMoved; }
        [[nodiscard]] EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "MouseMoved"; }

    private:
        float x_, y_;
    };

    class MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(const float x_offset, const float y_offset)
            : x_offset_(x_offset), y_offset_(y_offset) {}

        [[nodiscard]] float x_offset() const { return x_offset_; }
        [[nodiscard]] float y_offset() const { return y_offset_; }

        static EventType get_static_type() { return EventType::MouseScrolled; }
        [[nodiscard]] EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "MouseScrolled"; }

    private:
        float x_offset_, y_offset_;
    };

    class MouseButtonEvent : public Event {
    public:
        [[nodiscard]] int button() const { return button_; }

    protected:
        explicit MouseButtonEvent(const int button) : button_(button) {}
        int button_ = 0;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent {
    public:
        explicit MouseButtonPressedEvent(const int button) : MouseButtonEvent(button) {}

        static EventType get_static_type() { return EventType::MouseButtonPressed; }
        [[nodiscard]] EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "MouseButtonPressed"; }
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent {
    public:
        explicit MouseButtonReleasedEvent(const int button) : MouseButtonEvent(button) {}

        static EventType get_static_type() { return EventType::MouseButtonReleased; }
        [[nodiscard]] EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "MouseButtonReleased"; }
    };
}
