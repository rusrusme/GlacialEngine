//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "Menubar.hpp"

#include "gui/Settings.hpp"
#include "platform/FileDialogAPI.hpp"
#include "platform/ShellAPI.hpp"

namespace {
    //
    // Helper Functions
    //
    void update_window_title(const std::string& path) {
        std::string scene_name = std::filesystem::path(path).stem().string();
        engine::Application::get().window().set_title("Engine - " + scene_name);
    }

    //
    // Project Handlers
    //
    void handle_new_project(const engine::SystemContext& context) {
        std::string selected_file = engine::platform::FileDialogAPI::save_file(
            engine::platform::FileDialogFilter::PROJECT, "project");
        if (selected_file.empty()) return;

        // TODO: Initialize a new project file/structure at selected_file
        // engine::Project::create_new(selected_file);
    }

    void handle_open_project(const engine::SystemContext& context) {
        std::string selected_file = engine::platform::FileDialogAPI::open_file(
            engine::platform::FileDialogFilter::PROJECT);
        if (selected_file.empty()) return;

        // TODO: Load project configuration and active scene
        // engine::Project::load(selected_file);
    }

    void handle_save_project(const engine::SystemContext& context) {
        // TODO: Save active project settings
        // engine::Project::save_active();
    }

    void handle_save_project_as(const engine::SystemContext& context) {
        std::string selected_file = engine::platform::FileDialogAPI::save_file(
            engine::platform::FileDialogFilter::PROJECT, "basis");
        if (selected_file.empty()) return;

        // TODO: Serialize project settings to new location
        // engine::Project::save_as(selected_file);
    }

    //
    // Scene Handlers
    //
    void handle_new_scene(const std::shared_ptr<engine::Scene>& scene, engine::AssetManager& assets) {
        if (!scene) return;
        engine::SceneSerializer serializer(scene);
        serializer.deserialize("assets/scenes/clean.scene", assets);
        engine::Application::get().window().set_title(engine::setting::ENGINE_TITLE_);
    }

    void handle_open_scene(const std::shared_ptr<engine::Scene>& scene, engine::AssetManager& assets) {
        if (!scene) return;
        std::string selected_file = engine::platform::FileDialogAPI::open_file(engine::platform::FileDialogFilter::SCENE);
        if (selected_file.empty()) return;

        engine::SceneSerializer serializer(scene);
        serializer.deserialize(selected_file, assets);

        std::filesystem::path path(selected_file);
        if (path.filename() != "clean.scene") {
            update_window_title(path.generic_string());
        }
    }

    void handle_save_scene_as(const std::shared_ptr<engine::Scene>& scene, engine::AssetManager& assets) {
        if (!scene) return;
        std::string selected_file = engine::platform::FileDialogAPI::save_file(engine::platform::FileDialogFilter::SCENE, "scene");
        if (selected_file.empty()) return;

        engine::SceneSerializer serializer(scene);
        serializer.serialize(selected_file, assets);

        std::filesystem::path path(selected_file);
        if (path.filename() != "clean.scene") {
            update_window_title(path.generic_string());
        }
    }

    void handle_save_scene(const std::shared_ptr<engine::Scene>& scene, engine::AssetManager& assets) {
        if (!scene) return;
        const std::string& current_path = scene->get_path();
        if (!current_path.empty() && std::filesystem::path(current_path).filename() != "clean.scene") {
            engine::SceneSerializer serializer(scene);
            serializer.serialize(current_path, assets);
            update_window_title(current_path);
        }
        else {
            handle_save_scene_as(scene, assets);
        }
    }

    //
    // Settings Handler
    //
    void handle_open_settings(editor::Settings& settings) {
        settings.open();
    }
}

namespace editor {
    Menubar::Menubar() = default;
    Menubar::~Menubar() = default;

    #ifdef ENABLE_IMGUI
    void Menubar::file_menu(const engine::SystemContext& context, Settings& settings) {
        const auto scene = context.scenes.active_scene();
        auto& assets = context.assets;

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project...")) {
                handle_new_project(context);
            }
            if (ImGui::MenuItem("Save Project")) {
                handle_save_project(context);
            }
            if (ImGui::MenuItem("Save Project As...")) {
                handle_save_project_as(context);
            }
            if (ImGui::MenuItem("Open Project...")) {
                handle_open_project(context);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                handle_new_scene(scene, assets);
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                handle_save_scene(scene, assets);
            }
            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
                handle_save_scene_as(scene, assets);
            }
            if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) {
                handle_open_scene(scene, assets);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Settings...", "Ctrl+Alt+S")) {
                handle_open_settings(settings);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit")) {
                engine::Application::get().close();
            }

            ImGui::EndMenu();
        }
    }

    void Menubar::edit_menu(const engine::SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        auto& assets = context.assets;

        if (ImGui::BeginMenu("Edit")) {
            ImGui::EndMenu();
        }
    }

    void Menubar::window_menu(const engine::SystemContext& context) {
        if (ImGui::BeginMenu("Window")) {
            ImGui::EndMenu();
        }
    }

    void Menubar::help_menu(const engine::SystemContext& context) {
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation")) {
                engine::platform::ShellAPI::open_url("https://github.com/rusrusme/BasisEngine/wiki");
            }
            ImGui::EndMenu();
        }
    }

    void Menubar::render_imgui(const engine::SystemContext& context, Settings& settings) {
        const auto scene = context.scenes.active_scene();
        auto& assets = context.assets;

        const bool is_ctrl = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL)
            || engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);
        const bool is_shift = engine::Input::is_key_pressed(engine::Key::LEFT_SHIFT)
            || engine::Input::is_key_pressed(engine::Key::RIGHT_SHIFT);
        const bool is_alt = engine::Input::is_key_pressed(engine::Key::LEFT_ALT)
            || engine::Input::is_key_pressed(engine::Key::RIGHT_ALT);

        if (is_ctrl && !engine::Input::is_keyboard_captured() && !ImGuizmo::IsUsing()) {
            if (is_shift) {
                if (engine::Input::is_key_pressed_once(engine::Key::S)) {
                    handle_save_scene_as(scene, assets);
                }
            }
            else if (is_alt) {
                if (engine::Input::is_key_pressed_once(engine::Key::S)) {
                    handle_open_settings(settings);
                }
            }
            else {
                if (engine::Input::is_key_pressed_once(engine::Key::N)) {
                    handle_new_scene(scene, assets);
                }
                if (engine::Input::is_key_pressed_once(engine::Key::S)) {
                    handle_save_scene(scene, assets);
                }
                if (engine::Input::is_key_pressed_once(engine::Key::O)) {
                    handle_open_scene(scene, assets);
                }
            }
        }

        if (ImGui::BeginMainMenuBar()) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
            file_menu(context, settings);
            edit_menu(context);
            window_menu(context);
            help_menu(context);
            ImGui::PopStyleVar();
            ImGui::EndMainMenuBar();
        }
    }
    #endif
}
