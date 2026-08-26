//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorLayer.hpp"

#include "Config.hpp"

namespace editor {
    EditorLayer::EditorLayer(std::shared_ptr<engine::Scene> scene, std::shared_ptr<engine::AssetManager> asset_manager) : active_scene_(std::move(scene)), asset_manager_(std::move(asset_manager)) {}

    void EditorLayer::on_attach(engine::Renderer& renderer) {
        framebuffer_ = std::make_unique<engine::Framebuffer>(1280, 720);
        debug_panel_ = std::make_unique<DebugPanel>();
        editor_grid_.init(renderer);

        active_scene_->on_init(renderer);

        camera_.apply_default_view();
        camera_controller_.reset_mouse_state();

        const uint32_t width = engine::Application::get().window().get_width();
        const uint32_t height = engine::Application::get().window().get_height();
        camera_.set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
        camera_.set_projection_params(45.0f, 0.1f, 2000.0f);

        asset_manager_->scan_directory("assets");

        setting::config::load();
        const int toggle_debug_panel = setting::config::get_int("toggle_debug_panel");
        debug_panel_->set_debug_panel_enabled(toggle_debug_panel);
    }

    void EditorLayer::on_detach() {
        if (active_scene_) {
            active_scene_->on_shutdown();
        }
        setting::config::set_int("toggle_debug_panel", debug_panel_->is_debug_panel_enabled());
        setting::config::save();
    }

    void EditorLayer::on_update(const float delta_time) {
        if (engine::Input::is_key_pressed_once(engine::Key::F3)) {
            if (debug_panel_) debug_panel_->toggle_debug_panel();
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

        if (active_scene_) {
            active_scene_->on_render(renderer, camera_, *asset_manager_, delta_time);
        }
        if (debug_panel_ && debug_panel_->is_grid_enabled()) {
            editor_grid_.render(renderer);
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
        debug_panel_->render_imgui(renderer, delta_time);
    }
    #endif

    void EditorLayer::on_event(engine::Event& event) {
        engine::EventDispatcher dispatcher(event);

        // dispatcher.dispatch<engine::KeyPressedEvent>([](const engine::KeyPressedEvent& e) {
        //     CLIENT_TRACE("Pressed Key: {0}", e.key_code());
        //     return false;
        // });

        // dispatcher.dispatch<engine::MouseButtonPressedEvent>([](const engine::MouseButtonPressedEvent& e) {
        //     CLIENT_TRACE("Clicked Button: {0}", e.button());
        //     return false;
        // });

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

    void EditorLayer::resize_viewport(uint32_t width, uint32_t height) {
        if (framebuffer_) {
            framebuffer_->resize(width, height);
            camera_.set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
        }
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
