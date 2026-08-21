//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "Event.hpp"

namespace engine {
    class WindowClosedEvent : public Event {
    public:
        WindowClosedEvent() = default;

        static EventType get_static_type() { return EventType::WindowClosed; }
        [[nodiscard]] EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "WindowClosed"; }
    };

    class WindowResizedEvent : public Event {
    public:
        WindowResizedEvent(const int width, const int height) : width_(width), height_(height) {}

        [[nodiscard]] int width() const { return width_; }
        [[nodiscard]] int height() const { return height_; }

        static EventType get_static_type() { return EventType::WindowResized; }
        [[nodiscard]] EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "WindowResized"; }

    private:
        int width_, height_;
    };
}
