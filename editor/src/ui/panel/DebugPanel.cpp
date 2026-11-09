//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "DebugPanel.hpp"

#include "Config.hpp"
#include "core/Application.hpp"
#include "core/Config.hpp"
#include "ui/panel/ViewportPanel.hpp"

namespace editor {
    float DebugPanel::s_timer = 0.0f;
    float DebugPanel::s_fps_display = 0.0f;

    void DebugPanel::on_update(SystemContext& context, float delta_time) {
        s_timer += delta_time;
        if (s_timer >= engine::config::video::FPS_REFRESH_RATE_IN_SECONDS || s_fps_display == 0.0f) {
            s_fps_display = ImGui::GetIO().Framerate;
            s_timer = 0.0f;
        }
    }

    void DebugPanel::on_render_ui(SystemContext& context) {
        if (!visible_) return;

        ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin("Debug");
        ImGui::Text("Performance");
        ImGui::Separator();
        ImGui::Text("FPS: %.1f", s_fps_display);
        ImGui::Text("Frame Time: %.2f ms", 1000.0f / (s_fps_display > 0.0f ? s_fps_display : 1.0f));
        ImGui::Separator();
        ImGui::Text("Rendering");
        // const auto& [draw_calls, instance_count] = renderer.get_stats();
        // ImGui::Text("Draw Calls: %u", draw_calls);
        // ImGui::Text("Instances Rendered: %u", instance_count);
        ImGui::Separator();
        ImGui::Text("Video");
        bool vsync_enabled = Application::get().window().vsync();
        if (ImGui::Checkbox("V-Sync", &vsync_enabled)) {
            Application::get().window().set_vsync(vsync_enabled);
        }
        ImGui::Text("FPS Limit: %s", engine::config::video::target_fps_str().c_str());
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::SliderInt("##",
                         &engine::config::video::target_fps_index,
                         0,
                         engine::config::video::num_presets - 1,
                         engine::config::video::target_fps_str().c_str());
        ImGui::Separator();
        ImGui::Text("Editor");
        auto& viewport = context.get<SceneViewport>();
        ImGui::Checkbox("Grid", &viewport.show_grid);
        ImGui::End();
    }

    void DebugPanel::load_config() {
        visible_ = config::get_value("DebugPanel_visible", visible_);
    }

    void DebugPanel::save_config() {
        config::set_value("DebugPanel_visible", visible_);
    }
}
