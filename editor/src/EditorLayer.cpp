//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorLayer.hpp"

namespace editor {
    EditorLayer::EditorLayer(std::shared_ptr<engine::Scene> scene, std::shared_ptr<engine::AssetManager> asset_manager) : active_scene_(std::move(scene)), asset_manager_(std::move(asset_manager)) {}

    void EditorLayer::on_attach(engine::Renderer& renderer) {
        framebuffer_ = std::make_unique<engine::Framebuffer>(1280, 720);
        editor_grid_.init(renderer);

        active_scene_->on_init(renderer);

        camera_.apply_default_view();
        camera_controller_.reset_mouse_state();

        const uint32_t width = engine::Application::get().window().get_width();
        const uint32_t height = engine::Application::get().window().get_height();
        camera_.set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
        camera_.set_projection_params(45.0f, 0.1f, 10000.0f);

        asset_manager_->scan_directory("assets/models");
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

        const bool can_control_camera = is_viewport_hovered_ || is_viewport_focused_ ||
            engine::Input::is_mouse_button_pressed(engine::Mouse::MOUSE_BUTTON_MIDDLE);

        if (can_control_camera) {
            camera_controller_.update(camera_, delta_time);
        }
        else {
            camera_controller_.reset_mouse_state();
        }
        total_time_ += delta_time;
    }

    void EditorLayer::on_render(engine::Renderer& renderer, const float delta_time) {
        if (!framebuffer_) return;
        framebuffer_->bind();

        renderer.clear({0.12f, 0.12f, 0.12f, 1.0f});

        if (show_grid_) {
            editor_grid_.render(renderer);
        }
        if (active_scene_) {
            active_scene_->on_render(renderer, camera_, *asset_manager_, delta_time);
        }
        if (active_scene_) {
            auto view = active_scene_->view<engine::TransformComponent, engine::BoxColliderComponent>();

            for (auto entity_handle : view) {
                auto [transform, collider] = view.get<engine::TransformComponent, engine::BoxColliderComponent>(entity_handle);
                if (!collider.show_hitbox) continue;

                // Calculate World Space AABB
                engine::AABB world_aabb = collider.get_world_aabb(transform.position, transform.scale);
                auto overlaps = active_scene_->get_collision_system().query_aabb(*active_scene_, world_aabb);

                // Turn RED if colliding with another entity (> 1 because it includes itself), otherwise GREEN
                glm::vec3 box_color = (overlaps.size() > 1) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
                renderer.draw_box(world_aabb.min, world_aabb.max, box_color);
            }
        }
        auto dir_light = active_scene_->view<engine::TransformComponent, engine::DirectionalLightComponent>();
        for (auto entity : dir_light) {
            auto [transform, light] = dir_light.get<engine::TransformComponent, engine::DirectionalLightComponent>(entity);
            draw_directional_light(renderer, transform.position, transform.get_forward(), light.color);
        }
        auto point_light = active_scene_->view<engine::TransformComponent, engine::PointLightComponent>();
        for (auto entity : point_light) {
            auto [transform, light] = point_light.get<engine::TransformComponent, engine::PointLightComponent>(entity);
            renderer.draw_sphere(transform.position, 0.05f, light.color, 16);
        }
        framebuffer_->unbind();
    }

    #ifdef ENABLE_IMGUI
    void EditorLayer::on_imgui_render(engine::Renderer& renderer, const float delta_time) {
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
            ImGui::Text("Rendering");
            const auto& [draw_calls, instance_count] = renderer.get_stats();
            ImGui::Text("Draw Calls: %u", draw_calls);
            ImGui::Text("Instances Rendered: %u", instance_count);

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
            ImGui::End();
        }

        // ----------------------
        // Shader library inspector
        // ----------------------
        if (ImGui::Begin("Shader Library Inspector")) {
            const auto& shaders = renderer.get_shader_library().get_shaders();

            ImGui::Text("Registered Shaders: %zu", shaders.size());
            ImGui::Separator();

            static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;

            if (ImGui::BeginTable("ShaderLibraryTable", 3, flags)) {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Program ID", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableHeadersRow();

                for (const auto& [name, shader] : shaders) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    if (shader) {
                        ImGui::Text("%u", shader->get_renderer_id());
                    }
                    else {
                        ImGui::TextDisabled("-");
                    }
                    ImGui::TableSetColumnIndex(2);
                    if (shader) {
                        ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "Active");
                    }
                    else {
                        ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "Null");
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
    #endif

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
            if (is_viewport_hovered_) {
                engine::CameraController::on_scroll(camera_, e.y_offset());
                return true;
            }
            return false;
        });
    }

    void EditorLayer::draw_directional_light(engine::Renderer& renderer, const glm::vec3 position, const glm::vec3& direction, const glm::vec3& color) {
        const float ray_length = 1.5f;
        const float circle_radius = 0.05f;
        const glm::vec3 dir = glm::normalize(direction);

        // 1. Kleine Sonne im Zentrum zeichnen
        renderer.draw_sphere(position, circle_radius, color, 16);

        // 2. Haupt-Richtungsstrahl (Pfeil)
        const glm::vec3 main_target = position + dir * ray_length;
        renderer.draw_line(position, main_target, color);

        // 3. Pfeilspitze am Ende
        glm::vec3 up = std::abs(dir.z) < 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
        glm::vec3 right = glm::normalize(glm::cross(dir, up)) * 0.15f;
        up = glm::normalize(glm::cross(right, dir)) * 0.15f;

        renderer.draw_line(main_target, main_target - dir * 0.3f + right, color);
        renderer.draw_line(main_target, main_target - dir * 0.3f - right, color);
        renderer.draw_line(main_target, main_target - dir * 0.3f + up, color);
        renderer.draw_line(main_target, main_target - dir * 0.3f - up, color);
    }
}
