//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "Menubar.hpp"

#include "ProjectWizard.hpp"
#include "gui/Settings.hpp"
#include "misc/StressTests.hpp"
#include "platform/FileDialogAPI.hpp"
#include "platform/ShellAPI.hpp"
#include "project/Project.hpp"

namespace {
    void handle_save_scene(const std::shared_ptr<engine::Scene>& scene, engine::AssetManager& assets);

    //
    // Helper Functions
    //
    void update_window_title(const std::string& path) {
        const std::string scene_name = std::filesystem::path(path).stem().string();
        engine::Application::get().window().set_title("Engine - " + scene_name);
    }

    //
    // Project Handlers
    //
    void handle_new_project(engine::SystemContext& context) {
        const std::string selected_file = engine::platform::FileDialogAPI::save_file(
            engine::platform::FileDialogFilter::PROJECT, "glacial"
        );
        if (selected_file.empty()) return;

        const auto project = engine::Project::new_project(selected_file);
        if (!project) return;

        editor::ProjectWizard::setup(project);

        context.current_project = project;
        context.assets.clear_cache();
        context.assets.scan_directory(project->resolve_virtual_path("assets://"));
        context.scenes.create(engine::SceneManager::DEFAULT_SCENE);

        update_window_title(project->get_config().name);
    }

    void handle_open_project(engine::SystemContext& context) {
        const std::string selected_file = engine::platform::FileDialogAPI::open_file(
            engine::platform::FileDialogFilter::PROJECT
        );
        if (selected_file.empty()) return;

        const auto new_project = engine::Project::load_project(selected_file);
        if (!new_project) return;

        context.current_project = new_project;
        context.assets.clear_cache();
        context.assets.scan_directory(new_project->resolve_virtual_path("assets://"));

        bool loaded_initial = false;
        const auto& start_scene = new_project->get_config().initial_scene;

        if (!start_scene.empty()) {
            const auto resolved_scene_path = new_project->resolve_virtual_path(start_scene);
            if (std::filesystem::exists(resolved_scene_path)) {
                const auto scene = std::make_shared<engine::Scene>();
                engine::SceneSerializer serializer(scene);
                if (serializer.deserialize(resolved_scene_path.string(), context)) {
                    context.scenes.on_attach(scene);
                    loaded_initial = true;
                }
            }
        }
        if (!loaded_initial) {
            context.scenes.create(engine::SceneManager::DEFAULT_SCENE);
        }

        update_window_title(new_project->get_config().name);
    }

    void handle_save_project_as(engine::SystemContext& context) {
        if (!context.current_project) {
            handle_new_project(context);
            return;
        }

        const std::string selected_file = engine::platform::FileDialogAPI::save_file(
            engine::platform::FileDialogFilter::PROJECT, "basis");
        if (selected_file.empty()) return;

        if (context.current_project->save_as_project(selected_file)) {
            update_window_title(context.current_project->get_config().name);
        }
    }

    void handle_save_project(engine::SystemContext& context) {
        if (context.current_project) {
            context.current_project->save_project();

            // Save active scene alongside project settings
            if (const auto scene = context.scenes.active_scene()) {
                handle_save_scene(scene, context.assets);
            }
        }
        else {
            handle_save_project_as(context);
        }
    }

    void handle_close_project(engine::SystemContext& context) {
        if (!context.current_project) return;

        const auto name = context.current_project->get_config().name;
        context.current_project.reset();
        context.assets.clear_cache();
        context.assets.scan_directory("assets");
        context.scenes.create(engine::SceneManager::DEFAULT_SCENE);

        CLIENT_TRACE("Closed project {0}", name.c_str());
        engine::Application::get().window().set_title(engine::setting::ENGINE_TITLE_);
    }

    //
    // Scene Handlers
    //
    void handle_new_scene(const std::shared_ptr<engine::Scene>& scene, const engine::SystemContext& context) {
        if (!scene) return;
        context.scenes.create(engine::SceneManager::DEFAULT_SCENE);
        engine::Application::get().window().set_title(engine::setting::ENGINE_TITLE_);
    }

    void handle_open_scene(const std::shared_ptr<engine::Scene>& scene, const engine::SystemContext& context) {
        if (!scene) return;
        const std::string selected_file = engine::platform::FileDialogAPI::open_file(engine::platform::FileDialogFilter::SCENE);
        if (selected_file.empty()) return;

        engine::SceneSerializer serializer(scene);
        serializer.deserialize(selected_file, context);

        const std::filesystem::path path(selected_file);
        update_window_title(path.generic_string());
    }

    void handle_save_scene_as(const std::shared_ptr<engine::Scene>& scene, engine::AssetManager& assets) {
        if (!scene) return;
        const std::string selected_file = engine::platform::FileDialogAPI::save_file(engine::platform::FileDialogFilter::SCENE, "scene");
        if (selected_file.empty()) return;

        engine::SceneSerializer serializer(scene);
        serializer.serialize(selected_file, assets);

        const std::filesystem::path path(selected_file);
        update_window_title(path.generic_string());
    }

    void handle_save_scene(const std::shared_ptr<engine::Scene>& scene, engine::AssetManager& assets) {
        if (!scene) return;
        if (!scene->get_path().empty()) {
            engine::SceneSerializer serializer(scene);
            serializer.serialize(scene->get_path(), assets);
            update_window_title(scene->get_path());
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
    void Menubar::file_menu(engine::SystemContext& context, Settings& settings) {
        const auto scene = context.scenes.active_scene();
        auto& assets = context.assets;
        const bool has_project = context.current_project != nullptr;

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project...")) {
                handle_new_project(context);
            }
            if (ImGui::MenuItem("Open Project...")) {
                handle_open_project(context);
            }
            if (ImGui::MenuItem("Save Project", nullptr, false, has_project)) {
                handle_save_project(context);
            }
            if (ImGui::MenuItem("Save Project As...", nullptr, false, has_project)) {
                handle_save_project_as(context);
            }
            if (ImGui::MenuItem("Close Project", nullptr, false, has_project)) {
                handle_close_project(context);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                handle_new_scene(scene, context);
            }
            if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) {
                handle_open_scene(scene, context);
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                handle_save_scene(scene, assets);
            }
            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
                handle_save_scene_as(scene, assets);
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

    void Menubar::edit_menu(engine::SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        auto& assets = context.assets;

        // Todo: Edit
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z")) {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X")) {
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {
            }
            ImGui::EndMenu();
        }
    }

    void Menubar::window_menu(engine::SystemContext& context) {
        if (ImGui::BeginMenu("Window")) {
            ImGui::EndMenu();
        }
    }

    void Menubar::help_menu(engine::SystemContext& context) {
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation")) {
                engine::platform::ShellAPI::open_url("https://github.com/rusrusme/GlacialEngine/wiki");
            }
            ImGui::EndMenu();
        }
    }

    void Menubar::debug_menu(engine::SystemContext& context) {
        auto scene = context.scenes.active_scene();

        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Generate 10k Random Entities")) {
                generate_large_test_scene(context, scene, 10000);
            }
            ImGui::EndMenu();
        }
    }

    void Menubar::render_imgui(engine::SystemContext& context, Settings& settings) {
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
                    handle_new_scene(scene, context);
                }
                if (engine::Input::is_key_pressed_once(engine::Key::S)) {
                    handle_save_scene(scene, assets);
                }
                if (engine::Input::is_key_pressed_once(engine::Key::O)) {
                    handle_open_scene(scene, context);
                }
            }
        }

        if (ImGui::BeginMainMenuBar()) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
            file_menu(context, settings);
            edit_menu(context);
            window_menu(context);
            help_menu(context);
            debug_menu(context);
            ImGui::PopStyleVar();
            ImGui::EndMainMenuBar();
        }
    }
    #endif
}
