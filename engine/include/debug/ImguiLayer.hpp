//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Layer.hpp"

namespace engine {
    class ImguiLayer : public Layer {
    public:
        ImguiLayer() : Layer("Imgui Layer") {}
        virtual ~ImguiLayer() override = default;

        virtual void on_attach(SystemContext& context) override {}
        virtual void on_detach(SystemContext& context) override {}

        virtual void on_update(SystemContext& context, float delta_time) override {}
        virtual void on_render(SystemContext& context, float delta_time) override {}
        virtual void on_event(SystemContext& context, Event& event) override {}
    };
}
