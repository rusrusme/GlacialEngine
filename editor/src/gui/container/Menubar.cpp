//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Menubar.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#ifdef _WIN32
namespace {
    std::string open_file_dialog(const char* filter = "Scene Files (*.scene)\0*.scene\0All Files (*.*)\0*.*\0") {
        OPENFILENAMEA ofn;
        char sz_file[260] = {0};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFile = sz_file;
        ofn.nMaxFile = sizeof(sz_file);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameA(&ofn) == TRUE) {
            return {ofn.lpstrFile};
        }
        return "";
    }

    std::string save_file_dialog(const char* filter = "Scene Files (*.scene)\0*.scene\0All Files (*.*)\0*.*\0") {
        OPENFILENAMEA ofn;
        char sz_file[260] = {0};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFile = sz_file;
        ofn.nMaxFile = sizeof(sz_file);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.lpstrDefExt = "scene";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
        if (GetSaveFileNameA(&ofn) == TRUE) {
            return {ofn.lpstrFile};
        }
        return "";
    }
}
#endif

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
            #ifdef _WIN32
            std::string selected_file = open_file_dialog();
            if (!selected_file.empty()) {
                engine::SceneSerializer serializer(active_scene);
                serializer.deserialize(selected_file, *asset_manager);
                if (std::filesystem::path(selected_file).filename() != "clean.scene") {
                    update_window_title(selected_file);
                }
            }
            #endif
        };

        auto handle_save_scene = [update_window_title, active_scene, asset_manager] {
            #ifdef _WIN32
            std::string selected_file = save_file_dialog();
            if (!selected_file.empty()) {
                engine::SceneSerializer serializer(active_scene);
                serializer.serialize(selected_file, *asset_manager);
                if (std::filesystem::path(selected_file).filename() != "clean.scene") {
                    update_window_title(selected_file);
                }
            }
            #endif
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
