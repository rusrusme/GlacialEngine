//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Titlebar.hpp"

#include "core/Application.hpp"
#include "event/EditorEvents.hpp"
#include "event/EventQueue.hpp"
#include "platform/ShellAPI.hpp"
#include "project/scene/SceneManager.hpp"
#include "ui/Icons.hpp"

namespace editor {
    void Titlebar::on_init(SystemContext& context) {
    }

    void Titlebar::on_render_ui(SystemContext& context) {
        auto& window = Application::get().window();
        window.set_titlebar_hovered(false);

        constexpr float target_bar_height = 32.0f;
        const float font_size = ImGui::GetFontSize();
        const float pad_y = std::floor(std::max(0.0f, (target_bar_height - font_size) * 0.5f));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, pad_y));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, pad_y));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        if (ImGui::BeginMainMenuBar()) {
            const float bar_width = ImGui::GetWindowWidth();
            const float bar_height = ImGui::GetWindowHeight();

            file_menu(context);
            edit_menu(context);
            window_menu(context);
            help_menu(context);

            const std::string& title = window.title();
            const float title_width = ImGui::CalcTextSize(title.c_str()).x;
            ImGui::SetCursorPosX((bar_width - title_width) * 0.5f);
            ImGui::TextDisabled("%s", title.c_str());

            // Buttons match full titlebar height
            constexpr float btn_width = 48.0f;
            const ImVec2 btn_size(btn_width, bar_height);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));

            // Minimize
            ImGui::SetCursorPos(ImVec2(bar_width - btn_width * 3.0f, 0.0f));
            if (draw_icon_button("##MinBtn", draw_minimize_icon, btn_size)) {
                window.minimize();
            }

            // Maximize / Restore
            ImGui::SetCursorPos(ImVec2(bar_width - btn_width * 2.0f, 0.0f));
            const bool is_max = window.maximized();
            if (draw_icon_button("##MaxBtn", is_max ? draw_restore_icon : draw_maximize_icon, btn_size)) {
                is_max ? window.restore() : window.maximize();
            }

            ImGui::PopStyleColor(2);

            // Close
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.88f, 0.18f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.12f, 0.12f, 1.0f));
            ImGui::SetCursorPos(ImVec2(bar_width - btn_width, 0.0f));
            if (draw_icon_button("##CloseBtn", draw_close_icon, btn_size)) {
                EventQueue::submit<EditorActionEvent>(Action::Exit);
            }

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();

            const bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)
                && !ImGui::IsAnyItemHovered();
            window.set_titlebar_hovered(hovered);

            ImGui::EndMainMenuBar();
        }

        ImGui::PopStyleVar(3);
    }

    void Titlebar::file_menu(SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        const bool has_project = context.current_project != nullptr;

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project...", Shortcuts::text(Action::NewProject).data())) {
                EventQueue::submit<EditorActionEvent>(Action::NewProject);
            }
            if (ImGui::MenuItem("Open Project...", Shortcuts::text(Action::OpenProject).data())) {
                EventQueue::submit<EditorActionEvent>(Action::OpenProject);
            }
            if (ImGui::MenuItem("Save Project", Shortcuts::text(Action::SaveProject).data(), false, has_project)) {
                EventQueue::submit<EditorActionEvent>(Action::SaveProject);
            }
            if (ImGui::MenuItem("Save Project As...", Shortcuts::text(Action::SaveProjectAs).data(), false, has_project)) {
                EventQueue::submit<EditorActionEvent>(Action::SaveProjectAs);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("New Scene", Shortcuts::text(Action::NewScene).data())) {
                EventQueue::submit<EditorActionEvent>(Action::NewScene);
            }
            if (ImGui::MenuItem("Open Scene...", Shortcuts::text(Action::OpenScene).data())) {
                EventQueue::submit<EditorActionEvent>(Action::OpenScene);
            }
            if (ImGui::MenuItem("Save Scene", Shortcuts::text(Action::SaveScene).data())) {
                EventQueue::submit<EditorActionEvent>(Action::SaveScene);
            }
            if (ImGui::MenuItem("Save Scene As...", Shortcuts::text(Action::SaveSceneAs).data())) {
                EventQueue::submit<EditorActionEvent>(Action::SaveSceneAs);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Settings...", Shortcuts::text(Action::ToggleSettings).data())) {
                EventQueue::submit<EditorActionEvent>(Action::ToggleSettings);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit")) {
                EventQueue::submit<EditorActionEvent>(Action::Exit);
            }

            ImGui::EndMenu();
        }
    }

    void Titlebar::edit_menu(SystemContext& context) {
        const auto scene = context.scenes.active_scene();

        // Todo: Edit
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", Shortcuts::text(Action::Undo).data())) {
                EventQueue::submit<EditorActionEvent>(Action::Undo);
            }
            if (ImGui::MenuItem("Redo", Shortcuts::text(Action::Redo).data())) {
                EventQueue::submit<EditorActionEvent>(Action::Redo);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Copy", Shortcuts::text(Action::Copy).data())) {
                EventQueue::submit<EditorActionEvent>(Action::Copy);
            }
            if (ImGui::MenuItem("Paste", Shortcuts::text(Action::Paste).data())) {
                EventQueue::submit<EditorActionEvent>(Action::Paste);
            }
            ImGui::EndMenu();
        }
    }

    void Titlebar::window_menu(SystemContext& context) {
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::BeginMenu("Panel")) {
                if (ImGui::MenuItem("Hierarchy", Shortcuts::text(Action::ToggleHierarchy).data())) {
                    EventQueue::submit<EditorActionEvent>(Action::ToggleHierarchy);
                }
                if (ImGui::MenuItem("Inspector", Shortcuts::text(Action::ToggleInspector).data())) {
                    EventQueue::submit<EditorActionEvent>(Action::ToggleInspector);
                }
                if (ImGui::MenuItem("Viewport", Shortcuts::text(Action::ToggleViewport).data())) {
                    EventQueue::submit<EditorActionEvent>(Action::ToggleViewport);
                }
                if (ImGui::MenuItem("Asset Browser", Shortcuts::text(Action::ToggleAssetBrowser).data())) {
                    EventQueue::submit<EditorActionEvent>(Action::ToggleAssetBrowser);
                }
                if (ImGui::MenuItem("Rime", Shortcuts::text(Action::ToggleRime).data())) {
                    EventQueue::submit<EditorActionEvent>(Action::ToggleRime);
                }
                if (ImGui::MenuItem("Console", Shortcuts::text(Action::ToggleConsole).data())) {
                    EventQueue::submit<EditorActionEvent>(Action::ToggleConsole);
                }
                if (ImGui::MenuItem("History", Shortcuts::text(Action::ToggleHistory).data())) {
                    EventQueue::submit<EditorActionEvent>(Action::ToggleHistory);
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Reset Layout")) {
                EventQueue::submit<EditorActionEvent>(Action::ResetLayout);
            }
            ImGui::EndMenu();
        }
    }

    void Titlebar::help_menu(SystemContext& context) {
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation")) {
                platform::ShellAPI::open_url("https://github.com/rusrusme/BasisEngine/wiki");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("About")) {
                // TODO add about window
            }
            ImGui::EndMenu();
        }
    }
}
