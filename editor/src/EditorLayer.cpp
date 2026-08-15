//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorLayer.hpp"

#include <core/Config.hpp>
#include <input/Input.hpp>
#include <scene/Entity.hpp>

namespace editor {
    void EditorLayer::on_attach(engine::Renderer& renderer) {
        active_scene_ = std::make_shared<engine::Scene>();
        active_scene_->on_init(renderer);


        auto entity = active_scene_->create_entity();
    }

    void EditorLayer::on_detach() {
        if (active_scene_) {
            active_scene_->on_shutdown();
        }
    }

    void EditorLayer::on_update(engine::Renderer& renderer, GLFWwindow* window, engine::CameraController& camera_controller, const float delta_time) {
        if (engine::Input::is_key_pressed_once(window, GLFW_KEY_F3)) {
            show_debug_panel_ = !show_debug_panel_;
        }
        if (active_scene_) {
            active_scene_->on_update(delta_time);
        }
    }

    void EditorLayer::on_render(engine::Renderer& renderer) {
        if (show_grid_) {
            render_grid(renderer);
        }
        if (show_gizmo_) {
            render_gizmo(renderer);
        }
        if (active_scene_) {
            active_scene_->on_render(renderer);
        }
    }

    void EditorLayer::on_imGuiRender(const float delta_time) {
        #ifdef ENABLE_IMGUI
        // ----------------------
        // Debug panel
        // ----------------------
        if (show_debug_panel_) {
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
            ImGui::Text("Video");
            static bool vsync_enabled = engine::setting::video::VSYNC_ENABLED;
            if (ImGui::Checkbox("V-Sync", &vsync_enabled)) {
                glfwSwapInterval(vsync_enabled ? 1 : 0);
            }
            ImGui::Text("FPS Limit: %s", engine::setting::video::get_target_fps_str().c_str());
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::SliderInt("##", &engine::setting::video::target_fps_index, 0, engine::setting::video::num_presets - 1, engine::setting::video::get_target_fps_str().c_str());

            ImGui::Separator();
            ImGui::Text("Editor");
            bool grid_active = is_grid_enabled();
            if (ImGui::Checkbox("Show World Grid", &grid_active)) {
                set_grid_enabled(grid_active);
            }

            bool gizmo_active = is_gizmo_enabled();
            if (ImGui::Checkbox("Show Axis Gizmo", &gizmo_active)) {
                set_gizmo_enabled(gizmo_active);
            }
            ImGui::End();
        }
        #endif
    }

    void EditorLayer::render_grid(engine::Renderer& renderer) {
        renderer.render_grid();
    }

    void EditorLayer::render_gizmo(engine::Renderer& renderer) {
        // World Origin Gizmo (X = Red, Y = Green, Z = Blue)
        renderer.draw_line({0, 0, 0}, {2, 0, 0}, {1, 0, 0});
        renderer.draw_line({0, 0, 0}, {0, 2, 0}, {0, 1, 0});
        renderer.draw_line({0, 0, 0}, {0, 0, 2}, {0, 0, 1});
    }
}
