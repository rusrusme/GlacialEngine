//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "gui/DebugPanel.hpp"

#include "Config.hpp"

namespace editor {
    #ifdef ENABLE_IMGUI
    void DebugPanel::load_config() {
        const int toggle_debug_panel = config::get_value("toggle_debug_panel", show_debug_panel_);
        const int show_grid = config::get_value("show_grid", show_grid_);
        show_debug_panel_ = toggle_debug_panel;
        show_grid_ = show_grid;
    }

    void DebugPanel::save_config() {
        config::set_value("toggle_debug_panel", show_debug_panel_);
        config::set_value("show_grid", show_grid_);
    }

    void DebugPanel::render_imgui(engine::Renderer& renderer, const float delta_time) {
        if (!show_debug_panel_) return;

        render_shader_library(renderer);
        render_debug_panel(renderer, delta_time);
    }

    void DebugPanel::render_debug_panel(engine::Renderer& renderer, const float delta_time) {
        static float timer = 0.0f;
        static float fps_display = 0.0f;

        timer += delta_time;
        if (timer >= engine::setting::video::FPS_REFRESH_RATE_IN_SECONDS || fps_display == 0.0f) {
            fps_display = ImGui::GetIO().Framerate;
            timer = 0.0f;
        }
        ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin("Debug Panel");
        ImGui::Text("Performance");
        ImGui::Text("  FPS: %.1f", fps_display);
        ImGui::Text("  Frame Time: %.2f ms", 1000.0f / (fps_display > 0.0f ? fps_display : 1.0f));

        ImGui::Separator();
        ImGui::Text("Rendering");
        const auto& [draw_calls, instance_count] = renderer.get_stats();
        ImGui::Text("Draw Calls: %u", draw_calls);
        ImGui::Text("Instances Rendered: %u", instance_count);

        ImGui::Separator();
        ImGui::Text("Video");
        bool vsync_enabled = engine::Application::get().window().is_vsync();
        if (ImGui::Checkbox("V-Sync", &vsync_enabled)) {
            engine::Application::get().window().set_vsync(vsync_enabled);
        }
        ImGui::Text("FPS Limit: %s", engine::setting::video::get_target_fps_str().c_str());
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::SliderInt("##", &engine::setting::video::target_fps_index, 0, engine::setting::video::num_presets - 1, engine::setting::video::get_target_fps_str().c_str());

        ImGui::Separator();
        ImGui::Text("Editor");
        bool grid_active = is_grid_enabled();
        if (ImGui::Checkbox("Show Grid", &grid_active)) {
            set_grid_enabled(grid_active);
        }
        ImGui::End();
    }

    void DebugPanel::render_shader_library(engine::Renderer& renderer) {
        if (ImGui::Begin("Shader Library Inspector")) {
            const auto& shaders = renderer.get_shaders();

            ImGui::Text("Registered Shaders: %zu", shaders.size());
            ImGui::Separator();

            static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;

            if (ImGui::BeginTable("ShaderLibraryTable", 3, flags)) {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Program ID", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableHeadersRow();

                for (const auto& [name, shader] : shaders) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    if (shader) {
                        ImGui::Text("%u", shader->get_renderer_id());
                    }
                    else {
                        ImGui::TextDisabled("-");
                    }
                    ImGui::TableSetColumnIndex(2);
                    if (shader) {
                        ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "Active");
                    }
                    else {
                        ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "Null");
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
    #endif
}
