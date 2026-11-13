//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "debug/ImguiLayer.hpp"

namespace engine {
    void ImguiLayer::on_attach(SystemContext& context) {
        #ifdef ENABLE_IMGUI
        ImguiSystem::on_attach(context.window.native());
        s_font = static_cast<ImFont*>(ImguiSystem::s_font);
        #endif
    }

    void ImguiLayer::on_render(SystemContext& context, float delta_time) {
        #ifdef ENABLE_IMGUI
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
        #endif
    }
}
