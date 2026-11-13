//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/vec4.hpp>

#include "core/Layer.hpp"
#include "core/Window.hpp"
#include "debug/ImguiSystem.hpp"
#include "render/rhi/RenderContext.hpp"

struct ImFont;

namespace engine {
    class Event;
    struct SystemContext;

    class ImguiLayer : public Layer {
    public:
        ImguiLayer() : Layer("Imgui Layer") {}
        explicit ImguiLayer(std::string_view name) : Layer(name.data()) {}
        ~ImguiLayer() override = default;

        void on_attach(SystemContext& context) override;

        void on_detach(SystemContext& context) override { ImguiSystem::on_detach(); }
        void on_update(SystemContext& context, float delta_time) override {}

        void on_render(SystemContext& context, float delta_time) override;

        void on_event(SystemContext& context, Event& event) override { ImguiSystem::on_event(context, event); }

        void set_clear_backbuffer(bool enable) { clear_depth_and_color_ = enable; }
        void set_clear_color(const glm::vec4& color) { clear_color_ = color; }
        [[nodiscard]] const glm::vec4& clear_color() const noexcept { return clear_color_; }

    protected:
        /// Override this function to render Imgui
        virtual void on_imgui_render(SystemContext& context, float delta_time) {}

        [[nodiscard]] static ImFont* font() noexcept { return s_font; }

    private:
        bool clear_depth_and_color_{false}; // Default to false for overlays
        glm::vec4 clear_color_{0.10f, 0.10f, 0.11f, 1.0f};

        inline static ImFont* s_font = nullptr;
    };
}
