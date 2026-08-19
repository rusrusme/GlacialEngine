//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorUILayer.hpp"

#include <imGUi.h>
#include <imGuizmo.h>
#include <core/Application.hpp>
#include <event/MouseEvents.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <input/Input.hpp>
#include <input/KeyCodes.hpp>
#include <input/MouseCodes.hpp>
#include <physics/Slab.hpp>

namespace editor {
    namespace {
        ImGuizmo::OPERATION current_gizmo_operation_ = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE current_gizmo_mode_ = ImGuizmo::LOCAL;
    }

    EditorUILayer::EditorUILayer(const std::shared_ptr<engine::Scene>& scene) : active_scene_(scene) {}

    void EditorUILayer::on_attach(engine::Renderer& renderer) {
        object_editor_.init(active_scene_);
        hierarchy_.init(active_scene_);

        ImGuizmo::Style& style = ImGuizmo::GetStyle();
        style.TranslationLineThickness = 3.0f;
        style.TranslationLineArrowSize = 5.0f;
        style.CenterCircleSize = 4.0f;
        style.HatchedAxisLineThickness = 0.0f;
    }

    void EditorUILayer::on_detach() {
        object_editor_.init(nullptr);
        hierarchy_.init(nullptr);
    }

    void EditorUILayer::on_update(float delta_time) {
        if (engine::Input::is_keyboard_captured() || ImGuizmo::IsUsing()) {
            return;
        }
        const bool is_ctrl = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) || engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);

        if (is_ctrl) {
            if (engine::Input::is_key_pressed_once(engine::Key::W)) {
                current_gizmo_operation_ = ImGuizmo::TRANSLATE;
            }
            if (engine::Input::is_key_pressed_once(engine::Key::E)) {
                current_gizmo_operation_ = ImGuizmo::ROTATE;
            }
            if (engine::Input::is_key_pressed_once(engine::Key::R)) {
                current_gizmo_operation_ = ImGuizmo::SCALE;
            }
        }
    }

    void EditorUILayer::on_render(engine::Renderer& renderer, float delta_time) {
    }

    void EditorUILayer::on_imGuiRender(engine::Renderer& renderer, float delta_time) {
        ImGuizmo::BeginFrame();
        ImGuizmo::Enable(true);

        hierarchy_.render_ui();
        object_editor_.set_selected_entity(hierarchy_.get_selected_entity());
        object_editor_.render_ui();

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImVec2 viewport_pos = main_viewport->Pos;
        ImVec2 viewport_size = main_viewport->Size;

        if (camera_) {
            draw_gizmo(hierarchy_.get_selected_entity(), *camera_, viewport_pos, viewport_size);
        }
    }

    void EditorUILayer::on_event(engine::Event& event) {
        engine::EventDispatcher dispatcher(event);

        if (ImGuizmo::IsOver() || ImGuizmo::IsUsing()) {
            dispatcher.dispatch<engine::MouseButtonPressedEvent>([](engine::MouseButtonPressedEvent& _) {
                return true;
            });
            dispatcher.dispatch<engine::MouseMovedEvent>([](engine::MouseMovedEvent& _) {
                return true;
            });
            return;
        }

        dispatcher.dispatch<engine::MouseButtonPressedEvent>([this](engine::MouseButtonPressedEvent& e) {
            if (e.button() != engine::Mouse::MOUSE_BUTTON_LEFT) {
                return false;
            }
            if (!camera_ || engine::Input::is_mouse_captured()) {
                return false;
            }
            const glm::vec2 mouse_pos = engine::Input::get_mouse_position();
            const uint32_t width = engine::Application::get().window().get_width();
            const uint32_t height = engine::Application::get().window().get_height();
            const glm::vec2 viewport_size = {static_cast<float>(width), static_cast<float>(height)};
            const engine::Ray ray = camera_->screen_point_to_ray(mouse_pos, viewport_size);

            engine::Entity closest_entity{};
            float closest_distance = std::numeric_limits<float>::max();

            auto view = active_scene_->get_registry().view<engine::TransformComponent>();
            for (auto entity_handle : view) {
                engine::Entity entity{entity_handle, active_scene_.get()};
                auto& transform = entity.get_component<engine::TransformComponent>();

                // Lokale AABB (-0.5 bis 0.5) mit Position & Skalierung der Entity in Welt-AABB umrechnen
                AABB local_aabb{glm::vec3(-0.5f), glm::vec3(0.5f)};
                AABB world_aabb;

                glm::vec3 min_scaled = local_aabb.min * transform.scale;
                glm::vec3 max_scaled = local_aabb.max * transform.scale;

                world_aabb.min = transform.position + glm::min(min_scaled, max_scaled);
                world_aabb.max = transform.position + glm::max(min_scaled, max_scaled);

                float hit_distance = 0.0f;
                if (ray_intersects_aabb(ray, world_aabb, hit_distance)) {
                    if (hit_distance < closest_distance) {
                        closest_distance = hit_distance;
                        closest_entity = entity;
                    }
                }
            }
            hierarchy_.set_selected_entity(closest_entity);
            block_gizmo_until_mouse_up_ = true;
            return false;
        });
    }

    void EditorUILayer::draw_gizmo(engine::Entity selected_entity, const engine::Camera& camera, const ImVec2& viewport_pos, const ImVec2& viewport_size) {
        if (!selected_entity || !selected_entity.has_component<engine::TransformComponent>()) {
            return;
        }

        if (block_gizmo_until_mouse_up_) {
            if (!engine::Input::is_mouse_button_pressed(engine::Mouse::MOUSE_BUTTON_LEFT)) {
                block_gizmo_until_mouse_up_ = false;
            }
            ImGuizmo::Enable(false);
        }
        else {
            ImGuizmo::Enable(true);
        }

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
        ImGuizmo::SetRect(viewport_pos.x, viewport_pos.y, viewport_size.x, viewport_size.y);
        ImGuizmo::SetID(0);
        ImGuizmo::SetGizmoSizeClipSpace(0.115f);

        glm::mat4 view_matrix = camera.get_view_matrix();
        glm::mat4 proj_matrix = camera.get_projection_matrix();

        auto& transform = selected_entity.get_component<engine::TransformComponent>();

        if (ImGuizmo::IsUsing()) {
            if (!was_using_gizmo_) {
                initial_transform_ = transform;
                active_gizmo_matrix_ = transform.get_matrix();
                was_using_gizmo_ = true;
                gizmo_canceled_ = false;
            }
        }
        else {
            was_using_gizmo_ = false;
            gizmo_canceled_ = false;
        }

        glm::mat4 transform_matrix = was_using_gizmo_ ? active_gizmo_matrix_ : transform.get_matrix();

        bool snap = engine::Input::is_key_pressed(engine::Key::LEFT_CONTROL) || engine::Input::is_key_pressed(engine::Key::RIGHT_CONTROL);
        float snap_value = current_gizmo_operation_ == ImGuizmo::ROTATE ? 45.0f : 0.5f;
        float snap_values[3] = {snap_value, snap_value, snap_value};

        ImGuizmo::Manipulate(
            glm::value_ptr(view_matrix),
            glm::value_ptr(proj_matrix),
            current_gizmo_operation_,
            current_gizmo_mode_,
            glm::value_ptr(transform_matrix),
            nullptr,
            snap ? snap_values : nullptr
        );

        if (ImGuizmo::IsUsing()) {
            active_gizmo_matrix_ = transform_matrix;

            if (engine::Input::is_key_pressed(engine::Key::ESCAPE)) {
                gizmo_canceled_ = true;
            }

            if (gizmo_canceled_) {
                transform = initial_transform_;
            }
            else {
                float translation[3], rotation[3], scale[3];
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(transform_matrix),
                    translation,
                    rotation,
                    scale
                );

                if (current_gizmo_operation_ == ImGuizmo::TRANSLATE) {
                    transform.position = glm::make_vec3(translation);
                }
                else if (current_gizmo_operation_ == ImGuizmo::ROTATE) {
                    transform.rotation = glm::make_vec3(rotation);
                }
                else if (current_gizmo_operation_ == ImGuizmo::SCALE) {
                    transform.scale = glm::make_vec3(scale);
                }
                else {
                    transform.position = glm::make_vec3(translation);
                    transform.rotation = glm::make_vec3(rotation);
                    transform.scale = glm::make_vec3(scale);
                }
            }
        }
    }
}
