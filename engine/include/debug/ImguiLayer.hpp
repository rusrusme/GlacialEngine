//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Window.hpp"
#include "debug/ImguiSystem.hpp"
#include "glm/vec4.hpp"
#include "render/rhi/RenderContext.hpp"

namespace engine {
    class Event;
    struct SystemContext;

    class ImguiLayer : public Layer {
    public:
        ImguiLayer() : Layer("Imgui Layer") {}
        explicit ImguiLayer(std::string_view name) : Layer(name.data()) {}
        ~ImguiLayer() override = default;

        void on_attach(SystemContext& context) override {
            ImguiSystem::on_attach(context.window.native());
            s_font = ImguiSystem::s_font;
        }

        void on_detach(SystemContext& context) override { ImguiSystem::on_detach(); }
        void on_update(SystemContext& context, float delta_time) override {}

        void on_render(SystemContext& context, float delta_time) override {
            // Only clear if explicitly requested (e.g. Editor UI root)
            // Overlays sitting on top of a 3D scene must not clear the backbuffer
            if (clear_depth_and_color_) {
                auto& cb = context.render_context.active_command_buffer();
                cb.begin_render_pass(context.render_context.swapchain().backbuffer(), clear_color_);
                cb.end_render_pass();
            }

            ImguiSystem::begin_frame();
            on_imgui_render(context, delta_time);
            ImguiSystem::end_frame();
        }

        void on_event(SystemContext& context, Event& event) override {
            ImguiSystem::on_event(context, event);
        }

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
