//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "Event.hpp"

namespace engine {
    class KeyEvent : public Event {
    public:
        [[nodiscard]] int key_code() const { return key_code_; }

    protected:
        explicit KeyEvent(const int key_code) : key_code_(key_code) {}
        int key_code_ = 0;
    };

    class KeyPressedEvent : public KeyEvent {
    public:
        explicit KeyPressedEvent(const int key_code, const bool is_repeat = false)
            : KeyEvent(key_code), is_repeat_(is_repeat) {}

        [[nodiscard]] bool is_repeat() const { return is_repeat_; }

        static EventType get_static_type() { return EventType::KeyPressed; }
        [[nodiscard]] EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "KeyPressed"; }

    private:
        bool is_repeat_ = false;
    };

    class KeyReleasedEvent : public KeyEvent {
    public:
        explicit KeyReleasedEvent(const int key_code) : KeyEvent(key_code) {}

        static EventType get_static_type() { return EventType::KeyReleased; }
        [[nodiscard]] EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "KeyReleased"; }
    };

    class KeyTypedEvent : public KeyEvent {
    public:
        explicit KeyTypedEvent(const int key_code) : KeyEvent(key_code) {}

        static EventType get_static_type() { return EventType::KeyTyped; }
        [[nodiscard]] EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "KeyTyped"; }
    };
}
