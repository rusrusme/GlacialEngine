//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Menubar.hpp"

#include "platform/FileDialogAPI.hpp"

namespace editor {
    Menubar::Menubar() = default;
    Menubar::~Menubar() = default;

    #ifdef ENABLE_IMGUI
    void Menubar::render_imgui(const std::shared_ptr<engine::Scene>& active_scene, const std::shared_ptr<engine::AssetManager>& asset_manager) {
        auto update_window_title = [](const std::string& path) {
            std::string scene_name = std::filesystem::path(path).stem().string();
            engine::Application::get().window().set_title("Engine - " + scene_name);
        };

        const bool is_ctrl = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) ||
            engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);

        auto handle_new_scene = [active_scene, asset_manager] {
            engine::SceneSerializer serializer(active_scene);
            serializer.deserialize("assets/scenes/clean.scene", *asset_manager);
            engine::Application::get().window().set_title(engine::setting::FRAME_TITLE);
        };

        auto handle_load_scene = [update_window_title, active_scene, asset_manager] {
            std::string selected_file = engine::platform::FileDialogAPI::open_file(engine::platform::FileDialogFilter::SCENE);
            if (selected_file.empty()) return;
            engine::SceneSerializer serializer(active_scene);
            serializer.deserialize(selected_file, *asset_manager);
            std::filesystem::path path(selected_file);
            if (path.filename() != "clean.scene") {
                update_window_title(path.generic_string());
            }
        };

        auto handle_save_scene = [update_window_title, active_scene, asset_manager] {
            std::string selected_file = engine::platform::FileDialogAPI::save_file(engine::platform::FileDialogFilter::SCENE, "scene");
            if (selected_file.empty()) return;
            engine::SceneSerializer serializer(active_scene);
            serializer.serialize(selected_file, *asset_manager);
            std::filesystem::path path(selected_file);
            if (path.filename() != "clean.scene") {
                update_window_title(path.generic_string());
            }
        };

        if (is_ctrl && !engine::Input::is_keyboard_captured() && !ImGuizmo::IsUsing()) {
            if (engine::Input::is_key_pressed_once(engine::Key::N)) {
                handle_new_scene();
            }
            if (engine::Input::is_key_pressed_once(engine::Key::S)) {
                handle_save_scene();
            }
            if (engine::Input::is_key_pressed_once(engine::Key::O)) {
                handle_load_scene();
            }
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                    handle_new_scene();
                }
                if (ImGui::MenuItem("Save Scene As", "Ctrl+S")) {
                    handle_save_scene();
                }
                if (ImGui::MenuItem("Load Scene", "Ctrl+O")) {
                    handle_load_scene();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
    #endif
}
