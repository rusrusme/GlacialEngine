//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

#include "event/Event.hpp"
#include "event/EventType.hpp"

namespace editor {
    using namespace engine;

    class EntitySelectedEvent : public Event {
    public:
        explicit EntitySelectedEvent(uint64_t entity_id) : entity_id_(entity_id) {}

        [[nodiscard]] uint64_t entity_id() const { return entity_id_; }

        static engine::EventType get_static_type() { return static_cast<engine::EventType>(EventType::EntitySelected); }
        [[nodiscard]] engine::EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "EntitySelected"; }

    private:
        uint64_t entity_id_;
    };

    class EditorActionEvent : public Event {
    public:
        explicit EditorActionEvent(EditorAction action) : action_(action) {}

        [[nodiscard]] EditorAction action() const { return action_; }

        static engine::EventType get_static_type() { return static_cast<engine::EventType>(EventType::ActionTriggered); }
        [[nodiscard]] engine::EventType get_event_type() const override { return get_static_type(); }
        [[nodiscard]] const char* get_name() const override { return "ActionTriggered"; }

    private:
        EditorAction action_;
    };
}
