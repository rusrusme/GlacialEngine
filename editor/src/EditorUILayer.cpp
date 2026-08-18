//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorUILayer.hpp"

#include <imGUi.h>
#include <imGuizmo.h>
#include <event/MouseEvents.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <input/Input.hpp>
#include <input/KeyCodes.hpp>

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
        if (ImGuizmo::IsOver() || ImGuizmo::IsUsing()) {
            engine::EventDispatcher dispatcher(event);
            dispatcher.dispatch<engine::MouseButtonPressedEvent>([](engine::MouseButtonPressedEvent& _) {
                return true;
            });
            dispatcher.dispatch<engine::MouseMovedEvent>([](engine::MouseMovedEvent& _) {
                return true;
            });
        }
    }

    void EditorUILayer::draw_gizmo(engine::Entity selected_entity, const engine::Camera& camera, const ImVec2& viewport_pos, const ImVec2& viewport_size) {
        if (!selected_entity || !selected_entity.has_component<engine::TransformComponent>()) {
            return;
        }

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
        ImGuizmo::SetRect(viewport_pos.x, viewport_pos.y, viewport_size.x, viewport_size.y);
        ImGuizmo::SetID(0);
        ImGuizmo::SetGizmoSizeClipSpace(0.115f);

        glm::mat4 view_matrix = camera.get_view_matrix();
        glm::mat4 proj_matrix = camera.get_projection_matrix();

        auto& transform = selected_entity.get_component<engine::TransformComponent>();
        glm::mat4 transform_matrix = transform.get_matrix();

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
            if (!was_using_gizmo_) {
                initial_transform_ = transform;
                was_using_gizmo_ = true;
                gizmo_canceled_ = false;
            }

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

                transform.position = glm::make_vec3(translation);
                transform.rotation = glm::make_vec3(rotation);
                transform.scale = glm::make_vec3(scale);
            }
        }
        else {
            was_using_gizmo_ = false;
            gizmo_canceled_ = false;
        }
    }
}
