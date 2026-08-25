//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "DebugPanel.hpp"

namespace editor {
    #ifdef ENABLE_IMGUI
    void DebugPanel::render_imgui(engine::Renderer& renderer, float delta_time) {
        if (!show_debug_panel_) return;

        // ----------------------
        // Shader library inspector
        // ----------------------
        if (ImGui::Begin("Shader Library Inspector")) {
            const auto& shaders = renderer.get_shader_library().get_shaders();

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

        // ----------------------
        // Debug panel
        // ----------------------
        static float timer = 0.0f;
        static float fps_display = 0.0f;

        timer += delta_time;
        if (timer >= engine::setting::video::FPS_REFRESH_RATE_IN_SECONDS || fps_display == 0.0f) {
            fps_display = ImGui::GetIO().Framerate;
            timer = 0.0f;
        }
        ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin("Debug panel");
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
        static bool vsync_enabled = engine::Application::get().window().is_vsync();
        if (ImGui::Checkbox("V-Sync", &vsync_enabled)) {
            glfwSwapInterval(vsync_enabled ? 1 : 0);
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
    #endif
}
