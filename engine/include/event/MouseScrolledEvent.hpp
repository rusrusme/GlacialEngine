//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include "Event.hpp"

namespace engine {
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
}
