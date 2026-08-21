//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>

#include "event/Event.hpp"
#include "render/Renderer.hpp"

namespace engine {
    class Layer {
    public:
        explicit Layer(std::string name = "Layer") : debug_name_(std::move(name)) {}
        virtual ~Layer() = default;

        virtual void on_attach(Renderer& renderer) {}
        virtual void on_detach() {}
        virtual void on_update(float delta_time) {}
        virtual void on_render(Renderer& renderer, float delta_time) {}
        virtual void on_imgui_render(Renderer& renderer, float delta_time) {}

        virtual void on_event(Event& event) {}

    protected:
        std::string debug_name_;
    };
};
