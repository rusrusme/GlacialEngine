//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    // TODO create a solid api to add more EventTypes (not hardcoded enum)
    enum class EventType {
        None = 0,
        WindowResized, WindowClosed, WindowMoved,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
    };

    class Event {
    public:
        virtual ~Event() = default;

        bool handled = false;

        [[nodiscard]] virtual EventType get_event_type() const = 0;
        [[nodiscard]] virtual const char* get_name() const = 0;
    };

    class EventDispatcher {
    public:
        explicit EventDispatcher(Event& event) : event_(event) {}

        // TODO maybe cast to int so that custom events outside of engine could also use this
        template<typename T, typename F>
        bool dispatch(const F& function) {
            // if (event_.get_event_type() == T::get_static_type()) {
            if (static_cast<int>(event_.get_event_type()) == static_cast<int>(T::get_static_type())) {
                event_.handled |= function(static_cast<T&>(event_));
                return true;
            }
            return false;
        }

    private:
        Event& event_;
    };
}
