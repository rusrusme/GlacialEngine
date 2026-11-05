//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Menubar.hpp"

#include "core/Application.hpp"
#include "event/EditorEvents.hpp"
#include "event/EventQueue.hpp"
#include "platform/ShellAPI.hpp"
#include "project/scene/SceneManager.hpp"

namespace editor {
    void Menubar::on_init(SystemContext& context) {
    }

    void Menubar::on_render_ui(SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        auto& assets = context.assets;

        if (ImGui::BeginMainMenuBar()) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
            file_menu(context);
            edit_menu(context);
            window_menu(context);
            help_menu(context);
            ImGui::PopStyleVar();
            ImGui::EndMainMenuBar();
        }
    }

    void Menubar::file_menu(SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        auto& assets = context.assets;
        const bool has_project = context.current_project != nullptr;

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project...", Shortcuts::text(Action::NewProject).data())) {
            }
            if (ImGui::MenuItem("Open Project...", Shortcuts::text(Action::OpenProject).data())) {
            }
            if (ImGui::MenuItem("Save Project", Shortcuts::text(Action::SaveProject).data(), false, has_project)) {
            }
            if (ImGui::MenuItem("Save Project As...", Shortcuts::text(Action::SaveProjectAs).data(), false, has_project)) {
            }

            ImGui::Separator();

            if (ImGui::MenuItem("New Scene", Shortcuts::text(Action::NewScene).data())) {
            }
            if (ImGui::MenuItem("Open Scene...", Shortcuts::text(Action::OpenScene).data())) {
            }
            if (ImGui::MenuItem("Save Scene", Shortcuts::text(Action::SaveScene).data())) {
            }
            if (ImGui::MenuItem("Save Scene As...", Shortcuts::text(Action::SaveSceneAs).data())) {
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Settings...", Shortcuts::text(Action::OpenSettings).data())) {
                EventQueue::submit<EditorActionEvent>(Action::OpenSettings);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit")) {
                EventQueue::submit<EditorActionEvent>(Action::Exit);
            }

            ImGui::EndMenu();
        }
    }

    void Menubar::edit_menu(SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        auto& assets = context.assets;

        // Todo: Edit
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", Shortcuts::text(Action::Undo).data())) {
            }
            if (ImGui::MenuItem("Redo", Shortcuts::text(Action::Redo).data())) {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Copy", Shortcuts::text(Action::Copy).data())) {
            }
            if (ImGui::MenuItem("Paste", Shortcuts::text(Action::Paste).data())) {
            }
            ImGui::EndMenu();
        }
    }

    void Menubar::window_menu(SystemContext& context) {
        if (ImGui::BeginMenu("Window")) {
            ImGui::EndMenu();
        }
    }

    void Menubar::help_menu(SystemContext& context) {
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation")) {
                platform::ShellAPI::open_url("https://github.com/rusrusme/BasisEngine/wiki");
            }
            ImGui::EndMenu();
        }
    }
}
