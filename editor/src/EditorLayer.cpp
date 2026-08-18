//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorLayer.hpp"

#include <core/Application.hpp>
#include <core/Config.hpp>
#include <../../engine/include/input/KeyCodes.hpp>
#include <core/Log.hpp>
#include <event/KeyEvents.hpp>
#include <event/MouseEvents.hpp>
#include <event/WindowEvents.hpp>
#include <input/Input.hpp>
#include <scene/Entity.hpp>

namespace editor {
    void EditorLayer::on_attach(engine::Renderer& renderer) {
        editor_grid_.init();

        active_scene_ = std::make_shared<engine::Scene>();
        active_scene_->on_init(renderer);

        camera_.apply_default_view();
        camera_controller_.reset_mouse_state();

        const uint32_t width = engine::Application::get().window().get_width();
        const uint32_t height = engine::Application::get().window().get_height();
        camera_.set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
        camera_.set_projection_params(45.0f, 1.0f, 10000.0f);
    }

    void EditorLayer::on_detach() {
        if (active_scene_) {
            active_scene_->on_shutdown();
        }
    }

    void EditorLayer::on_update(const float delta_time) {
        if (engine::Input::is_key_pressed_once(engine::Key::F3)) {
            show_debug_panel_ = !show_debug_panel_;
        }
        if (active_scene_) {
            active_scene_->on_update(delta_time);
        }
        if (!engine::Input::is_mouse_captured()) {
            camera_controller_.update(camera_, delta_time);
        }
        else {
            camera_controller_.reset_mouse_state();
        }
        total_time_ += delta_time;
    }

    void EditorLayer::on_render(engine::Renderer& renderer, const float delta_time) {
        if (show_grid_) {
            editor_grid_.render(renderer);
        }
        if (show_gizmo_) {
            // World Origin Gizmo (X = Red, Y = Green, Z = Blue)
            renderer.draw_line({0, 0, 0}, {2, 0, 0}, {1, 0, 0});
            renderer.draw_line({0, 0, 0}, {0, 2, 0}, {0, 1, 0});
            renderer.draw_line({0, 0, 0}, {0, 0, 2}, {0, 0, 1});
        }
        if (active_scene_) {
            active_scene_->on_render(renderer, camera_, delta_time);
        }
        // --------------------------------
        // 1. Big cube grid test
        // --------------------------------
        cube_grid(renderer);

        // --------------------------------
        // 2. Orbiting spheres test
        // --------------------------------
        orbiting_spheres(renderer);

        // --------------------------------
        // 3. Dynamic rotating ring test
        // --------------------------------
        dynamic_ring(renderer);
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
            static bool vsync_enabled = engine::Application::get().window().is_vsync();
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

    void EditorLayer::on_event(engine::Event& event) {
        engine::EventDispatcher dispatcher(event);

        dispatcher.dispatch<engine::KeyPressedEvent>([](const engine::KeyPressedEvent& e) {
            CLIENT_TRACE("Pressed Key: {0}", e.key_code());
            return false;
        });

        dispatcher.dispatch<engine::MouseButtonPressedEvent>([](const engine::MouseButtonPressedEvent& e) {
            CLIENT_TRACE("Clicked Button: {0}", e.button());
            return false;
        });

        // dispatcher.dispatch<engine::MouseMovedEvent>([](const engine::MouseMovedEvent& e) {
        //     CLIENT_TRACE("Moved Mouse to: {0}, {1}", e.x(), e.y());
        //     return false;
        // });

        dispatcher.dispatch<engine::WindowResizedEvent>([this](const engine::WindowResizedEvent& e) {
            if (e.height() > 0) {
                camera_.set_aspect_ratio(static_cast<float>(e.width()) / static_cast<float>(e.height()));
            }
            return false;
        });

        dispatcher.dispatch<engine::MouseScrolledEvent>([this](const engine::MouseScrolledEvent& e) {
            if (!engine::Input::is_mouse_captured()) {
                engine::CameraController::on_scroll(camera_, e.y_offset());
                return true;
            }
            return false;
        });
    }

    void EditorLayer::cube_grid(engine::Renderer& renderer) const {
        constexpr int grid_x = 10;
        constexpr int grid_y = 10;
        constexpr int grid_z = 10;
        constexpr float spacing = 2.5f;

        for (int x = 0; x < grid_x; ++x) {
            for (int y = 0; y < grid_y; ++y) {
                for (int z = 0; z < grid_z; ++z) {
                    const float pos_x = (static_cast<float>(x) - grid_x * 0.5f) * spacing;
                    const float pos_y = static_cast<float>(y) * spacing + 1.0f;
                    const float pos_z = (static_cast<float>(z) - grid_z * 0.5f) * spacing;

                    // Rainbow color based on position and time
                    const float r = (std::sin(pos_x * 0.1f + total_time_) + 1.0f) * 0.5f;
                    const float g = (std::sin(pos_y * 0.2f + total_time_ * 1.5f) + 1.0f) * 0.5f;
                    const float b = (std::cos(pos_z * 0.1f + total_time_) + 1.0f) * 0.5f;

                    const glm::vec3 min = {pos_x - 0.8f, pos_y - 0.8f, pos_z - 0.8f};
                    const glm::vec3 max = {pos_x + 0.8f, pos_y + 0.8f, pos_z + 0.8f};

                    renderer.draw_box(min, max, {r, g, b});
                }
            }
        }
    }

    void EditorLayer::orbiting_spheres(engine::Renderer& renderer) const {
        constexpr int num_spheres = 12;
        for (int i = 0; i < num_spheres; ++i) {
            const float angle = (static_cast<float>(i) / num_spheres) * glm::two_pi<float>() + total_time_;
            constexpr float radius_orbit = 25.0f;

            const glm::vec3 center = {
                std::cos(angle) * radius_orbit,
                15.0f + std::sin(total_time_ * 2.0f + static_cast<float>(i)) * 5.0f,
                std::sin(angle) * radius_orbit
            };
            renderer.draw_sphere(center, 3.0f, {1.0f, 0.2f, 0.2f}, 64);
        }
    }

    void EditorLayer::dynamic_ring(engine::Renderer& renderer) const {
        constexpr glm::vec3 ring_center = {0.0f, 35.0f, 0.0f};
        const glm::vec3 ring_normal = glm::normalize(glm::vec3(
            std::sin(total_time_),
            std::cos(total_time_ * 0.5f),
            0.0f
        ));
        renderer.draw_circle(ring_center, 12.0f, ring_normal, {0.0f, 1.0f, 0.8f}, 128);
    }
}
