//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include "debug/ImguiSystem.hpp"

namespace engine {
    class Event;
    struct SystemContext;

    class ImguiLayer : public Layer {
    public:
        ImguiLayer() : Layer("Imgui Layer") {}
        ~ImguiLayer() override = default;

        void on_attach(SystemContext& context) final {
            ImguiSystem::on_attach(context.window.native());
        }

        void on_detach(SystemContext& context) final { ImguiSystem::on_detach(); }
        void on_update(SystemContext& context, float delta_time) override {}

        void on_render(SystemContext& context, float delta_time) final {
            ImguiSystem::begin_frame();
            on_imgui_render(context, delta_time);
            ImguiSystem::end_frame();
        }

        void on_event(SystemContext& context, Event& event) final {
            ImguiSystem::on_event(context, event);
        }

    protected:
        /// Override this function to render Imgui
        virtual void on_imgui_render(SystemContext& context, float delta_time) {}
    };
}
