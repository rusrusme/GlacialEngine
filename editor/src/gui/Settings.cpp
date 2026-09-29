//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Settings.hpp"

#include <cmath>

namespace editor {
    void Settings::on_update(const engine::SystemContext& context, float delta_time) {
        if (!auto_save_enabled_) {
            auto_save_timer_ = 0.0f;
            return;
        }

        auto_save_timer_ += delta_time;
        const float interval_seconds = static_cast<float>(auto_save_interval_minutes_) * 60.0f;

        if (auto_save_timer_ < interval_seconds) return;
        auto_save_timer_ = 0.0f;

        if (context.scenes.scene_state() != engine::SceneState::Edit) return;

        auto scene = context.scenes.active_scene();
        if (!scene) return;

        const std::string& current_path = scene->get_path();
        if (!current_path.empty() && std::filesystem::path(current_path).filename() != "clean.scene") {
            CLIENT_INFO("Auto saving...");
            engine::SceneSerializer serializer(scene);
            serializer.serialize(current_path, context.assets);
        }
    }

    #ifdef ENABLE_IMGUI
    void Settings::ui_scale() {
        int current_percent = static_cast<int>(std::lround(ui_scale_ * 100.0f));

        ImGui::AlignTextToFramePadding();
        ImGui::Text("UI Scale");
        ImGui::SameLine(150.0f);

        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const float total_width = 85.0f;
        const float arrow_width = 20.0f;
        const float height = ImGui::GetFrameHeight();

        ImGui::SetNextItemWidth(total_width);
        if (ImGui::InputScalar("##ui_scale_input", ImGuiDataType_S32, &current_percent, nullptr, nullptr, "%d%%")) {
            current_percent = std::clamp(current_percent, 25, 300);
            ui_scale_ = static_cast<float>(current_percent) / 100.0f;
            ImGui::GetIO().FontGlobalScale = ui_scale_;
        }

        const ImVec2 arrow_min(pos.x + total_width - arrow_width, pos.y);
        const ImVec2 arrow_max(pos.x + total_width, pos.y + height);
        const bool is_arrow_hovered = ImGui::IsMouseHoveringRect(arrow_min, arrow_max);

        if (is_arrow_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImGui::OpenPopup("##ui_scale_popup");
        }

        const ImU32 arrow_color = is_arrow_hovered
                                      ? ImGui::GetColorU32(ImGuiCol_Text)
                                      : ImGui::GetColorU32(ImGuiCol_TextDisabled);

        const ImVec2 center(pos.x + total_width - (arrow_width * 0.5f), pos.y + (height * 0.5f));
        const float arrow_size = 3.5f;

        ImGui::GetWindowDrawList()->AddTriangleFilled(
            ImVec2(center.x - arrow_size, center.y - (arrow_size * 0.5f)),
            ImVec2(center.x + arrow_size, center.y - (arrow_size * 0.5f)),
            ImVec2(center.x, center.y + (arrow_size * 0.7f)),
            arrow_color
        );

        ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + height + 2.0f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(total_width, 0.0f), ImVec2(total_width, 300.0f));

        if (ImGui::BeginPopup("##ui_scale_popup")) {
            static const int presets[] = {70, 80, 90, 100, 110, 125, 150, 175, 200};
            for (int preset : presets) {
                char label[32];
                snprintf(label, sizeof(label), "%d%%", preset);
                bool is_selected = (current_percent == preset);

                if (ImGui::Selectable(label, is_selected)) {
                    ui_scale_ = static_cast<float>(preset) / 100.0f;
                    ImGui::GetIO().FontGlobalScale = ui_scale_;
                    ImGui::CloseCurrentPopup();
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndPopup();
        }
    }

    void Settings::render_imgui(const engine::SystemContext& context) {
        if (!is_open_) return;

        // Center window on first appearance
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport) {
            ImGui::SetNextWindowPos(
                ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f, viewport->Pos.y + viewport->Size.y * 0.5f),
                ImGuiCond_FirstUseEver,
                ImVec2(0.5f, 0.5f)
            );
        }
        ImGui::SetNextWindowSize(ImVec2(600.0f, 400.0f), ImGuiCond_FirstUseEver);

        // Keep as a non-dockable floating window
        if (ImGui::Begin("Settings", &is_open_, ImGuiWindowFlags_NoDocking)) {
            if (ImGui::BeginTabBar("SettingsTabs")) {
                if (ImGui::BeginTabItem("General")) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("General Editor Settings");
                    ImGui::Separator();
                    ImGui::Spacing();

                    ui_scale();

                    ImGui::Spacing();

                    if (ImGui::Checkbox("Enable Auto-Save", &auto_save_enabled_)) {
                        auto_save_timer_ = 0.0f;
                    }

                    if (auto_save_enabled_) {
                        ImGui::SliderInt("Auto-Save Interval (minutes)", &auto_save_interval_minutes_, 1, 30);
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Graphics")) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("Viewport & Rendering Options");
                    ImGui::Separator();
                    ImGui::Spacing();

                    static float default_fov = 60.0f;
                    ImGui::SliderFloat("Default Camera FOV", &default_fov, 30.0f, 120.0f);

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Keybindings")) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("Editor Shortcuts");
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Text("New Scene:        Ctrl + N");
                    ImGui::Text("Open Scene:       Ctrl + O");
                    ImGui::Text("Save Scene:       Ctrl + S");
                    ImGui::Text("Save Scene As:    Ctrl + Shift + S");
                    ImGui::Text("Settings:         Ctrl + Alt + S");

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
    #endif
}
