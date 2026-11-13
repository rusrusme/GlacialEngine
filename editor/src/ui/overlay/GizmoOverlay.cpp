//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "GizmoOverlay.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "command/EditorCommands.hpp"
#include "command/Transaction.hpp"
#include "input/Input.hpp"
#include "input/KeyCodes.hpp"
#include "input/MouseCodes.hpp"
#include "render/camera/Camera.hpp"
#include "ui/panel/Hierarchy.hpp"
#include "ui/panel/ViewportPanel.hpp"

namespace editor {
    void GizmoOverlay::on_render_overlay(SystemContext& context) {
        if (!enabled_) {
            was_using_gizmo_ = false;
            return;
        }

        auto& selected_entity = context.get<SelectionContext>().selected_entity;
        if (!selected_entity || !selected_entity.has_component<TransformComponent>()) {
            was_using_gizmo_ = false;
            return;
        }

        draw_gizmo(context, selected_entity);
    }

    void GizmoOverlay::cancel_drag(SystemContext& context) {
        if (was_using_gizmo_ || ImGuizmo::IsUsing()) {
            gizmo_canceled_ = true;
            ImGui::ClearActiveID();

            auto& selected_entity = context.get<SelectionContext>().selected_entity;
            if (selected_entity && selected_entity.has_component<TransformComponent>()) {
                selected_entity.get_component<TransformComponent>() = initial_transform_;
            }
        }
    }

    void GizmoOverlay::draw_gizmo(SystemContext& context, Entity entity) {
        auto& viewport = context.get<SceneViewport>();
        auto& camera = context.get<Camera>();

        // Disable ImGuizmo during cancellation until the mouse button is released
        if (gizmo_canceled_) {
            ImGuizmo::Enable(false);
            if (!Input::mouse_button_pressed(Mouse::MOUSE_BUTTON_LEFT)) {
                gizmo_canceled_ = false;
                was_using_gizmo_ = false;
            }
        }
        else {
            ImGuizmo::Enable(true);
        }

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(viewport.bounds_min.x, viewport.bounds_min.y, viewport.size.x, viewport.size.y);

        const glm::mat4& view = camera.view();
        glm::mat4 projection = camera.projection();

        if (projection[1][1] < 0.0f) {
            projection[1][1] *= -1.0f;
        }

        auto& tc = entity.get_component<TransformComponent>();
        glm::mat4 transform = tc.get_matrix();

        bool snap = snap_enabled_ || Input::key_pressed(Key::LEFT_CONTROL);
        float* snap_values = (operation_ == ImGuizmo::ROTATE)
                                 ? &snap_rotation_value_
                                 : (operation_ == ImGuizmo::SCALE)
                                 ? &snap_scale_value_
                                 : snap_translation_values_;

        // Capture initial transform ONLY on the first frame of a new drag
        if (ImGuizmo::IsUsing() && !was_using_gizmo_ && !gizmo_canceled_) {
            initial_transform_ = tc;
            was_using_gizmo_ = true;
        }

        ImGuizmo::Manipulate(
            glm::value_ptr(view),
            glm::value_ptr(projection),
            operation_,
            mode_,
            glm::value_ptr(transform),
            nullptr,
            snap ? snap_values : nullptr
        );

        if (ImGuizmo::IsUsing() && !gizmo_canceled_) {
            glm::vec3 position, rotation, scale;

            ImGuizmo::DecomposeMatrixToComponents(
                glm::value_ptr(transform),
                glm::value_ptr(position),
                glm::value_ptr(rotation),
                glm::value_ptr(scale)
            );

            glm::vec3 delta_rotation = rotation - tc.rotation;
            tc.position = position;
            tc.rotation += delta_rotation;
            tc.scale = scale;
            tc.set_dirty();
        }

        // Reset drag tracking when mouse left button is fully released
        if (!ImGuizmo::IsUsing() && !Input::mouse_button_pressed(Mouse::MOUSE_BUTTON_LEFT)) {
            if (was_using_gizmo_ && !gizmo_canceled_) {
                if (initial_transform_.position != tc.position ||
                    initial_transform_.rotation != tc.rotation ||
                    initial_transform_.scale != tc.scale) {
                    auto& history = context.get<CommandHistory>();
                    Transaction tx("Transform Entity", history);
                    history.record(std::make_unique<ChangeTransformCommand>(entity.uuid(), initial_transform_, tc), context);
                    }
            }
            was_using_gizmo_ = false;
        }
    }
}
