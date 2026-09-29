//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Settings.hpp"

#include <cmath>

namespace editor {
    #ifdef ENABLE_IMGUI
    void Settings::ui_scale() {
        int current_percent = static_cast<int>(std::lround(ui_scale_ * 100.0f));

        ImGui::AlignTextToFramePadding();
        ImGui::Text("UI Scale");
        ImGui::SameLine(150.0f);

        const ImVec2 widget_start_pos = ImGui::GetCursorScreenPos();
        const float input_width = 75.0f;
        const float arrow_width = ImGui::GetFrameHeight();
        const float total_width = input_width + arrow_width;

        ImGui::SetNextItemWidth(input_width);
        if (ImGui::InputInt("##ui_scale_input", &current_percent, 0, 0, *"%d%%")) {
            current_percent = std::clamp(current_percent, 25, 300);
            ui_scale_ = static_cast<float>(current_percent) / 100.0f;
            ImGui::GetIO().FontGlobalScale = ui_scale_;
        }

        ImGui::SameLine(0.0f, 0.0f);

        const auto popup_pos = ImVec2(widget_start_pos.x, widget_start_pos.y + ImGui::GetFrameHeight());
        ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSizeConstraints(ImVec2(total_width, 0.0f), ImVec2(FLT_MAX, FLT_MAX));

        ImGui::SetNextItemWidth(arrow_width);
        if (ImGui::BeginCombo("##ui_scale_combo", nullptr, ImGuiComboFlags_NoPreview)) {
            static const int presets[] = {70, 80, 90, 100, 110, 125, 150, 175, 200};
            for (int preset : presets) {
                char label[32];
                snprintf(label, sizeof(label), "%d%%", preset);
                bool is_selected = (current_percent == preset);

                if (ImGui::Selectable(label, is_selected)) {
                    ui_scale_ = static_cast<float>(preset) / 100.0f;
                    ImGui::GetIO().FontGlobalScale = ui_scale_;
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
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

                    static bool auto_save = false;
                    ImGui::Checkbox("Enable Auto-Save", &auto_save);

                    static int autosave_interval = 5;
                    if (auto_save) {
                        ImGui::SliderInt("Auto-Save Interval (minutes)", &autosave_interval, 1, 30);
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
