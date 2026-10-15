//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>

#include "event/Event.hpp"

namespace engine {
    struct SystemContext;

    class Layer {
    public:
        explicit Layer(std::string name = "Layer") : name_(std::move(name)) {}
        virtual ~Layer() = default;

        virtual void on_attach(SystemContext& context) {}
        virtual void on_detach(SystemContext& context) {}

        virtual void on_update(SystemContext& context, float delta_time) {}
        virtual void on_render(SystemContext& context, float delta_time) {}
        virtual void on_event(SystemContext& context, Event& event) {}

    protected:
        std::string name_;
    };
};
