//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SettingsWindow.hpp"

#include "Config.hpp"
#include "EditorSettings.hpp"
#include "core/SystemContext.hpp"

namespace editor {
    void SettingsWindow::on_render_ui(SystemContext& context) {
        if (!visible_) return;

        if (ImGuiViewport* viewport = ImGui::GetMainViewport()) {
            ImGui::SetNextWindowPos(
                ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f, viewport->Pos.y + viewport->Size.y * 0.5f),
                ImGuiCond_FirstUseEver,
                ImVec2(0.5f, 0.5f)
            );
        }
        ImGui::SetNextWindowSize(ImVec2(600.0f, 400.0f), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Settings", &visible_, ImGuiWindowFlags_NoDocking)) {
            if (ImGui::BeginTabBar("SettingsTabs")) {
                auto& settings = context.get<EditorSettings>();
                for (const auto& category : settings.categories()) {
                    if (ImGui::BeginTabItem(category->name().data())) {
                        category->on_render_ui(context);
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    void SettingsWindow::load_config() {
        visible_ = false;
    }

    void SettingsWindow::save_config() {
        visible_ = false;
    }
}
